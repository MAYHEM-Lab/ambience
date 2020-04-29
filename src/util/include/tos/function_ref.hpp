//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/debug/panic.hpp>
#include <tos/utility.hpp>

namespace tos {
namespace detail {
template<bool Checked = false>
struct function_ref_base;

template<>
struct function_ref_base<false> {
    template<class RetT, class... ArgTs>
    function_ref_base(RetT (*fun)(ArgTs...), void* data)
        : m_function_ptr(reinterpret_cast<void (*)(void*)>(fun))
        , m_data(data) {
    }

    template<class RetT, class... ArgTs>
    auto pointer_as() const -> RetT (*)(ArgTs...) {
        return reinterpret_cast<RetT (*)(ArgTs...)>(m_function_ptr);
    }

    auto data() const -> const void* {
        return m_data;
    }

    auto data() -> void* {
        return m_data;
    }

private:
    void (*m_function_ptr)(void*);
    void* m_data;
};

struct dangerous_tag {};
} // namespace detail

template<class RetT, class... ArgTs>
class function_ref;

/**
 * This type implements a non owning reference to a
 * callable object, including raw function pointers.
 *
 * Usage:
 *     auto fr = function_ref<void()>([]{});
 *
 *     auto lambda = [&](int x) -> std::string { ... };
 *     auto fr = function_ref<std::string(int)>(lambda);
 *
 * When used with stateful function objects, it's
 * important to note that the constructor takes an
 * l-value reference, which means we cannot pass
 * temporaries to it.
 *
 * The converting constructor from a raw function
 * pointer is explicit to avoid unwanted constructions.
 */
template<class RetT, class... ArgTs>
class function_ref<RetT(ArgTs...)> : detail::function_ref_base<false> {
public:
    using internal_funptr_t = RetT (*)(ArgTs..., void*);

    function_ref(internal_funptr_t ptr, void* data)
        : function_ref_base<false>(ptr, data) {
        if (fun() == nullptr) {
            debug::panic("Function cannot be null!");
        }
    }

    explicit function_ref(internal_funptr_t ptr)
        : function_ref(ptr, nullptr) {
    }

    template<class T, std::enable_if_t<!std::is_same_v<T, function_ref>>* = nullptr>
    explicit function_ref(T& func)
        : function_ref(
              [](ArgTs... args, void* data) -> RetT {
                  static_assert(!std::is_const_v<T>, "Function cannot be a temporary!");
                  auto& actual_fun = *static_cast<T*>(data);
                  return actual_fun(std::forward<ArgTs>(args)...);
              },
              &func) {
    }

    template<class... CallArgTs>
    RetT operator()(CallArgTs&&... args) {
        return fun()(std::forward<CallArgTs>(args)..., data());
    }

    template<class... CallArgTs>
    RetT operator()(CallArgTs&&... args) const {
        return fun()(std::forward<CallArgTs>(args)..., data());
    }

    function_ref(detail::dangerous_tag, const detail::function_ref_base<false>& base)
        : function_ref_base<false>(base) {
    }

private:
    internal_funptr_t fun() {
        return pointer_as<RetT, ArgTs..., void*>();
    }

    template<class ToFun>
    friend ToFun unsafe_function_ref_cast(const function_ref& from) {
        return ToFun(detail::dangerous_tag{}, static_cast<const function_ref_base<false>&>(from));
    }
};

template<class ToFun, class RetT, class... ArgTs>
ToFun unsafe_function_ref_cast(const function_ref<RetT(ArgTs...)>& from);
} // namespace tos
