#pragma once

#include <tos/meta/function_traits.hpp>

namespace tos {
namespace detail {
template<class...>
class to_ptr;
template<class ClassT, class RetT, class... ArgTs>
class to_ptr<ClassT, RetT, meta::list<ArgTs...>> {
public:
    using ptr_type = RetT (*)(const ClassT&, ArgTs...);
};
} // namespace detail

template<auto MemberFun>
constexpr auto mem_fn() {
    using Traits = meta::function_traits<decltype(MemberFun)>;
    using Ptr = detail::
        to_ptr<typename Traits::class_t, typename Traits::ret_t, typename Traits::arg_ts>;
    if constexpr (Traits::is_const) {
        return static_cast<typename Ptr::ptr_type>(
            [](const typename Traits::class_t& instance, auto... args) -> decltype(auto) {
                return (instance.*MemberFun)(args...);
            });
    } else {
        return static_cast<typename Ptr::ptr_type>(
            [](typename Traits::class_t& instance, auto... args) -> decltype(auto) {
                return (instance.*MemberFun)(args...);
            });
    }
}
} // namespace tos