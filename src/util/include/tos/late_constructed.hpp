#pragma once

#include <memory>

namespace tos {
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
struct late_constructed {
    late_constructed() {
    }

    ~late_constructed() {
        std::destroy_at(&t);
    }

    template<class FnT, class... ArgTs>
    void emplace_fn(FnT&& fn, ArgTs&&... args) {
        new (&t) T(std::invoke(fn, std::forward<ArgTs>(args)...));
    }

    template<class... ArgTs>
    void emplace(ArgTs&&... args) {
        new (&t) T(std::forward<ArgTs>(args)...);
    }

    operator T&() {
        return t;
    }

    operator const T&() const {
        return t;
    }

    T& get() {
        return t;
    }

    const T& get() const {
        return t;
    }

private:
    union {
        T t;
    };
};
} // namespace tos