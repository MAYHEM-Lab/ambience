#pragma once

#include <lidlrt/builder.hpp>
#include <lidlrt/zerocopy_vtable.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/meta/algorithm.hpp>

namespace tos::ae {
template<class ServiceT, int ProcId>
struct procedure_decision {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    static inline constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
    using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
    using ArgsTupleType = typename lidl::detail::convert_types<
        typename ProcTraits::param_types>::tuple_type;

    static std::unique_ptr<quik::share_base> do_share(cur_arch::address_space& from_space,
                                                      cur_arch::address_space& to_space,
                                                      const void* args_tuple,
                                                      void* ret_ptr) {
        auto& typed_tuple = *static_cast<const ArgsTupleType*>(args_tuple);
        auto res = cur_arch::create_share(
            from_space, to_space, *physical_page_allocator::instance(), typed_tuple);
        return std::make_unique<decltype(res)>(std::move(res));
    }
};

struct sharer_vtbl {
    using do_share_fn =
        std::unique_ptr<quik::share_base> (*)(cur_arch::address_space& from_space,
                                              cur_arch::address_space& to_space,
                                              const void*,
                                              void*);

    do_share_fn do_share;
};

namespace detail {
template<class ServiceT, std::size_t... Is>
auto make_downcall_sharer(std::index_sequence<Is...>) {
    static constexpr sharer_vtbl vtbl[] = {
        sharer_vtbl{&procedure_decision<ServiceT, Is>::do_share}...};
    return tos::span<const sharer_vtbl>(vtbl);
}
} // namespace detail

template<class ServiceT>
auto make_downcall_sharer() {
    using ServDesc = lidl::service_descriptor<ServiceT>;

    return detail::make_downcall_sharer<ServiceT>(
        std::make_index_sequence<std::tuple_size_v<decltype(ServDesc::procedures)>>{});
}
} // namespace tos::ae