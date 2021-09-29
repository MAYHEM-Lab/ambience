#pragma once

#include <lidlrt/builder.hpp>
#include <lidlrt/zerocopy_vtable.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/meta/algorithm.hpp>

namespace tos::ae {
enum class ipc_policy
{
    copy,
    ro_share,
    rw_share,
};

struct always_copy_decision {
    template<class T>
    static constexpr ipc_policy value(T&) {
        return ipc_policy::copy;
    }

    template<class T>
    static constexpr size_t ipc_copy_size(const T& t) {
        return sizeof t;
    }
};

template<class T>
struct ipc_policy_decision;

template<>
struct ipc_policy_decision<uint32_t> : always_copy_decision {};

template<>
struct ipc_policy_decision<uint64_t> : always_copy_decision {};

template<>
struct ipc_policy_decision<int32_t> : always_copy_decision {};

template<>
struct ipc_policy_decision<int64_t> : always_copy_decision {};

template<>
struct ipc_policy_decision<bool> : always_copy_decision {};

template<>
struct ipc_policy_decision<std::string_view> {
    static ipc_policy value(const std::string_view& str) {
        if (str.size() < 32) {
            return ipc_policy::copy;
        }
        return ipc_policy::ro_share;
    }

    static constexpr size_t ipc_copy_size(const std::string_view& t) {
        return t.size();
    }
};

template<>
struct ipc_policy_decision<tos::span<uint8_t>> {
    static ipc_policy value(const tos::span<uint8_t>& data) {
        if (data.size() < 32) {
            return ipc_policy::copy;
        }
        return ipc_policy::ro_share;
    }
};

template<>
struct ipc_policy_decision<lidl::message_builder> {
    static ipc_policy value(const tos::span<uint8_t>& data) {
        return ipc_policy::rw_share;
    }

    static constexpr size_t ipc_copy_size(const lidl::message_builder& t) {
        return 0;
    }
};

template<class T>
constexpr ipc_policy decide_policy(const T& val) {
    using arg_t = std::remove_const_t<std::remove_reference_t<decltype(val)>>;
    return ipc_policy_decision<arg_t>::value(val);
}

template<class T>
constexpr size_t ipc_copy_size(const T& val) {
    using arg_t = std::remove_const_t<std::remove_reference_t<decltype(val)>>;
    return ipc_policy_decision<arg_t>::ipc_copy_size(val);
}

template<class ServiceT, int ProcId>
struct procedure_decision {
    using ServDesc = lidl::service_descriptor<ServiceT>;
    static inline constexpr auto& proc_desc = std::get<ProcId>(ServDesc::procedures);
    using ProcTraits = lidl::procedure_traits<decltype(proc_desc.function)>;
    using ArgsTupleType = typename lidl::detail::convert_types<
        typename ProcTraits::param_types>::tuple_type;

    static auto decide_policies(const ArgsTupleType& args_tuple) {
        return meta::transform(args_tuple,
                               [](auto* val_ptr) { return decide_policy(*val_ptr); });
    }

    static size_t ipc_area_size(const ArgsTupleType& args_tuple) {
        const auto ipc_sizes = meta::transform(
            args_tuple, [](auto* val_ptr) { return ipc_copy_size(*val_ptr); });
        const auto copy_size_sum = meta::accumulate(ipc_sizes, std::size_t(0));
        return copy_size_sum;
    }

    static size_t erased_ipc_area_size(const void* args_tuple_ptr) {
        return ipc_area_size(*static_cast<const ArgsTupleType*>(args_tuple_ptr));
    }

    static ArgsTupleType translate(const ArgsTupleType& args_tuple);

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

using do_share_fn =
    std::unique_ptr<quik::share_base> (*)(cur_arch::address_space& from_space,
                                          cur_arch::address_space& to_space,
                                          const void*,
                                          void*);
using ipc_area_compute_fn = size_t (*)(const void*);

struct sharer_vtbl {
    ipc_area_compute_fn area_compute;
    do_share_fn do_share;
};

namespace detail {
template<class ServiceT, std::size_t... Is>
auto make_downcall_sharer(std::index_sequence<Is...>) {
    static constexpr sharer_vtbl vtbl[] = {
        sharer_vtbl{&procedure_decision<ServiceT, Is>::erased_ipc_area_size,
                    &procedure_decision<ServiceT, Is>::do_share}...};
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