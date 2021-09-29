#pragma once

#include <functional>
#include <memory>

namespace tos {
template<class T, bool = std::is_reference_v<T>>
struct maybe_constructed {
    constexpr maybe_constructed() {
    }
    ~maybe_constructed() {
    }

    void destroy() {
        std::destroy_at(&t);
    }

    template<class FnT, class... ArgTs>
    constexpr void emplace_fn(FnT&& fn, ArgTs&&... args) {
        new (&t) T(std::invoke(fn, std::forward<ArgTs>(args)...));
    }

    template<class... ArgTs>
    constexpr void emplace(ArgTs&&... args) {
        new (&t) T(std::forward<ArgTs>(args)...);
    }

    constexpr operator T&() {
        return t;
    }

    constexpr operator const T&() const {
        return t;
    }

    constexpr T& get() {
        return t;
    }

    constexpr const T& get() const {
        return t;
    }

private:
    union {
        T t;
    };
};

template<class T>
struct maybe_constructed<T, true> {
private:
    using type = std::remove_reference_t<T>;

public:
    constexpr void destroy() {
    }

    template<class FnT, class... ArgTs>
    constexpr void emplace_fn(FnT&& fn, ArgTs&&... args) {
        emplace(std::invoke(fn, std::forward<ArgTs>(args)...));
    }

    constexpr void emplace(T arg) {
        m_internal.emplace(&arg);
    }

    constexpr operator T() {
        return static_cast<T>(*m_internal.get());
    }

    constexpr operator const T() const {
        return static_cast<T>(*m_internal.get());
    }

    constexpr T get() {
        return static_cast<T>(*m_internal.get());
    }

    constexpr const T get() const {
        return static_cast<T>(*m_internal.get());
    }

private:
    maybe_constructed<type*> m_internal;
};
/*
 * This template allows us to postpone the initialization of objects of types without
 * default constructors. For instance, a class member who cannot be initialized in a
 * constructor.
 *
 * However, unlike std::optional, objects of this type **must** be initialized, as their
 * destructors will be run unconditionally. If what you need is truly an optional member,
 * use std::optional.
 *
 * Example:
 *
 *      late_constructed<function_ref<void()>> fun_ref;
 *
 *      // At a later point
 *
 *      fun_ref.emplace(init_expr());
 *
 * However, if the object in question does not support moves or copies, this class
 * supports a workaround. Basically, it can evaluate a function and emplace it's result
 * in itself via mandatory copy elision.
 *
 * Example:
 *
 *      struct non_moveable;
 *
 *      non_moveable func(int);
 *
 *      late_constructed<non_moveable> no_moves;
 *
 *      // no_moves.emplace(func(42)); <- does not compile
 *      no_moves.emplace_fn(func, 42); // workaround
 */
template<class T>
struct late_constructed : public maybe_constructed<T> {
    ~late_constructed() {
        this->destroy();
    }
};
} // namespace tos