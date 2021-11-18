#pragma once

#include <lidlrt/meta.hpp>
#include <lidlrt/service.hpp>
#include <tos/out_ptr.hpp>

namespace lidl {
using zerocopy_fn_t = bool (*)(lidl::service_base&, const void* params, void* ret);
using async_zerocopy_fn_t = tos::Task<bool> (*)(lidl::service_base&,
                                                const void* params,
                                                void* ret);

namespace detail {
template<class>
struct convert_types;

template<class... Ts>
struct convert_types<lidl::meta::list<Ts...>> {
    using type = lidl::meta::list<zerocopy_type_t<Ts>...>;
    using tuple_type = std::tuple<zerocopy_type_t<Ts>...>;
};

template<class ServiceT, int ProcId>
struct zerocopy_translator {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    static constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
    using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
    using ArgsTupleType =
        typename convert_types<typename ProcTraits::param_types>::tuple_type;
    using RetType = typename ProcTraits::return_type;
    static constexpr bool is_ref = std::is_reference_v<RetType>;
    using ActualRetType =
        std::conditional_t<is_ref,
                           std::add_pointer_t<std::remove_reference_t<RetType>>,
                           RetType>;

    // using CallUnionType = typename ServiceT::wire_types::call_union;
    // using CallUnionTraits = union_traits<CallUnionType>;
    // using CallStructType =
    //     decltype(std::invoke(std::declval<const CallUnionType&>(),
    //                          std::get<ProcId>(CallUnionTraits::members).const_function));

    static auto async_typed(typename ServiceT::async_server& serv,
                            const ArgsTupleType* args,
                            tos::out_ptr<ActualRetType> ret) -> tos::Task<bool> {
        auto do_call = [&serv, ret](auto&&... vals) -> tos::Task<bool> {
            if constexpr (is_ref) {
                auto& res =
                    co_await std::invoke(proc_desc.async_function,
                                         serv,
                                         extractor<decltype(vals)>::extract(vals)...);
                new (ret) ActualRetType(&res);
            } else {
                new (ret) ActualRetType(
                    co_await std::invoke(proc_desc.async_function,
                                         serv,
                                         extractor<decltype(vals)>::extract(vals)...));
            }
            co_return true;
        };

        co_return co_await std::apply(do_call, *args);
    };

    static constexpr auto sync_typed(typename ServiceT::sync_server& serv,
                                     const ArgsTupleType* args,
                                     tos::out_ptr<ActualRetType> ret) -> bool {
        auto do_call = [&serv, ret](auto&&... vals) -> bool {
            if constexpr (is_ref) {
                auto& res = std::invoke(proc_desc.function,
                                        serv,
                                        extractor<decltype(vals)>::extract(vals)...);
                new (ret) ActualRetType(&res);
            } else {
                new (ret) ActualRetType(
                    std::invoke(proc_desc.function,
                                serv,
                                extractor<decltype(vals)>::extract(vals)...));
            }
            return true;
        };

        return std::apply(do_call, *args);
    };

    static auto async_untyped(lidl::service_base& serv_base,
                              const void* args,
                              tos::out_ptr<void> ret) -> tos::Task<bool> {
        return async_typed(static_cast<typename ServiceT::async_server&>(serv_base),
                           static_cast<const ArgsTupleType*>(args),
                           static_cast<tos::out_ptr<ActualRetType>>(ret));
    };

    static constexpr auto sync_untyped(lidl::service_base& serv_base,
                                       const void* args,
                                       tos::out_ptr<void> ret) -> bool {
        return sync_typed(static_cast<typename ServiceT::sync_server&>(serv_base),
                          static_cast<const ArgsTupleType*>(args),
                          static_cast<tos::out_ptr<ActualRetType>>(ret));
    };
};

template<class ServiceT, size_t... Is>
constexpr zerocopy_fn_t vt[] = {&zerocopy_translator<ServiceT, Is>::sync_untyped...};

template<class ServiceT, size_t... Is>
constexpr async_zerocopy_fn_t avt[] = {
    &zerocopy_translator<ServiceT, Is>::async_untyped...};

template<class ServiceT, size_t... Is>
constexpr tos::span<const zerocopy_fn_t>
do_make_zerocopy_vtable(std::index_sequence<Is...>) {
    return tos::span<const zerocopy_fn_t>(vt<ServiceT, Is...>);
}

template<class ServiceT, size_t... Is>
constexpr tos::span<const async_zerocopy_fn_t>
do_make_async_zerocopy_vtable(std::index_sequence<Is...>) {
    return tos::span<const async_zerocopy_fn_t>(avt<ServiceT, Is...>);
}
} // namespace detail

using zerocopy_vtable_t = tos::span<const zerocopy_fn_t>;
using async_zerocopy_vtable_t = tos::span<const async_zerocopy_fn_t>;

template<class ServiceT>
constexpr zerocopy_vtable_t make_zerocopy_vtable() {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    return detail::do_make_zerocopy_vtable<ServiceT>(
        std::make_index_sequence<std::tuple_size_v<decltype(ServDesc::procedures)>>{});
}

template<class ServiceT>
constexpr async_zerocopy_vtable_t make_async_zerocopy_vtable() {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    return detail::do_make_async_zerocopy_vtable<ServiceT>(
        std::make_index_sequence<std::tuple_size_v<decltype(ServDesc::procedures)>>{});
}

template<class T>
inline constexpr async_zerocopy_vtable_t async_vtable = make_async_zerocopy_vtable<T>();
} // namespace lidl