#pragma once

#include <lidlrt/meta.hpp>
#include <lidlrt/service.hpp>

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
    using type = lidl::meta::list<decltype(&std::declval<Ts&>())...>;
    using tuple_type = std::tuple<decltype(&std::declval<Ts&>())...>;
};

template<class ServiceT, int ProcId>
constexpr auto async_zerocopy_translator() -> async_zerocopy_fn_t {
    return [](lidl::service_base& serv_base,
              const void* args,
              void* ret) -> tos::Task<bool> {
        using ServDesc = lidl::service_descriptor<ServiceT>;
        constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
        using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
        using ArgsTupleType =
            typename convert_types<typename ProcTraits::param_types>::tuple_type;
        using RetType = typename ProcTraits::return_type;
        constexpr bool is_ref = std::is_reference_v<RetType>;
        using ActualRetType =
            std::conditional_t<is_ref,
                               std::add_pointer_t<std::remove_reference_t<RetType>>,
                               RetType>;
        constexpr auto& fn = proc_desc.async_function;

        auto do_call = [&serv = static_cast<typename ServiceT::async_server&>(serv_base),
                        ret](auto*... vals) -> tos::Task<bool> {
            if constexpr (is_ref) {
                auto& res = co_await std::invoke(fn, serv, *vals...);
                new (ret) ActualRetType(&res);
            } else {
                new (ret) ActualRetType(co_await std::invoke(fn, serv, *vals...));
            }
            co_return true;
        };

        auto& args_tuple = *static_cast<const ArgsTupleType*>(args);
        co_return co_await std::apply(do_call, args_tuple);
    };
}

template<class ServiceT, int ProcId>
constexpr auto zerocopy_translator() -> zerocopy_fn_t {
    return [](lidl::service_base& serv_base, const void* args, void* ret) -> bool {
        auto& serv = static_cast<typename ServiceT::sync_server&>(serv_base);
        using ServDesc = lidl::service_descriptor<ServiceT>;
        constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
        using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
        using ArgsTupleType =
            typename convert_types<typename ProcTraits::param_types>::tuple_type;
        using RetType = typename ProcTraits::return_type;
        static constexpr bool is_ref = std::is_reference_v<RetType>;
        using ActualRetType =
            std::conditional_t<is_ref,
                               std::add_pointer_t<std::remove_reference_t<RetType>>,
                               RetType>;

        auto do_call = [&serv, ret](auto*... vals) -> bool {
            constexpr auto& fn = proc_desc.function;
            if constexpr (is_ref) {
                auto& res = std::invoke(fn, serv, *vals...);
                new (ret) ActualRetType(&res);
            } else {
                new (ret) ActualRetType(std::invoke(fn, serv, *vals...));
            }
            return true;
        };

        auto& args_tuple = *static_cast<const ArgsTupleType*>(args);
        return std::apply(do_call, args_tuple);
    };
}

template<class ServiceT, size_t... Is>
constexpr zerocopy_fn_t vt[] = {zerocopy_translator<ServiceT, Is>()...};

template<class ServiceT, size_t... Is>
constexpr async_zerocopy_fn_t avt[] = {async_zerocopy_translator<ServiceT, Is>()...};

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
} // namespace lidl