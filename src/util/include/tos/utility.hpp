//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

#include <array>
#include <chrono>
#include <new>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <utility>

namespace tos {
template<class T>
using invoke_t = typename T::type;

template<class S1, class S2>
struct concat;

template<class T, T... I1, T... I2>
struct concat<std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>>
    : std::integer_sequence<T, I1..., (sizeof...(I1) + I2)...> {};

template<class S1, class S2>
using concat_t = invoke_t<concat<S1, S2>>;

struct non_copyable {
    non_copyable() = default;
    non_copyable(const non_copyable&) = delete;
    non_copyable(non_copyable&&) = default;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable& operator=(non_copyable&&) = default;
    ~non_copyable() = default;
};

struct non_movable {
    non_movable() = default;
    non_movable(const non_movable&) = default;
    non_movable(non_movable&&) = delete;
    non_movable& operator=(const non_movable&) = default;
    non_movable& operator=(non_movable&&) = delete;
    ~non_movable() = default;
};

struct non_copy_movable
    : non_copyable
    , non_movable {};

template<class... Fs>
struct overload : Fs... {
    template<class... Ts>
    overload(Ts&&... ts)
        : Fs{std::forward<Ts>(ts)}... {
    }

    using Fs::operator()...;
};

template<class... Ts>
auto make_overload(Ts&&... ts) {
    return overload<std::remove_reference_t<Ts>...>(std::forward<Ts>(ts)...);
}

/**
 * This type holds an object of type T, but prevents it's destructor from
 * being called upon destruction.
 *
 * It is useful for function static variables since they will try to call
 * std::atexit without being forgotten, which may not be available.
 *
 * WARNING: If you move-construct or copy-construct the internal object,
 * standard C++ rules still apply to the object you construct it with.
 * This means when used like the following:
 *
 *     auto f = forget(std::vector{1,2,3});
 *
 * The destructor of the vector in `f` won't be called, but the destructor
 * of the temporary vector will still be called!
 *
 * Inspired from rust's mem::forget, but inferior since we don't have
 * linear types or destructing moves.
 */
template<class T>
struct forget {
    forget() {
        new (get_ptr()) T;
    }

    template<class... Us>
    forget(Us&&... args) {
        new (get_ptr()) T(std::forward<Us>(args)...);
    }

    T& get() {
        return *std::launder(get_ptr());
    }

    operator T&() {
        return get();
    }

private:
    T* get_ptr() {
        return static_cast<T*>(static_cast<void*>(&storage));
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
};

template<class T>
forget(T&&) -> forget<T>;

template<class T, size_t N>
constexpr std::array<T, N - 1> pop_front(const std::array<T, N>& arr) {
    std::array<T, N - 1> res{};
    std::copy(arr.begin() + 1, arr.end(), res.begin());
    return res;
}

template<class T, size_t N>
constexpr std::array<T, N - 1> pop_back(const std::array<T, N>& arr) {
    std::array<T, N - 1> res{};
    std::copy(arr.begin(), arr.end() - 1, res.begin());
    return res;
}

template<class T>
constexpr T align_nearest_down_pow2(T val, size_t alignment) {
#if __has_builtin(__builtin_align_down)
    return __builtin_align_down(val, alignment);
#else
    return val & ~(alignment - 1);
#endif
}

template<class T>
constexpr T align_nearest_up_pow2(T val, size_t alignment) {
#if __has_builtin(__builtin_align_up)
    return __builtin_align_up(val, alignment);
#else
    return (val + alignment - 1) & ~(alignment - 1);
#endif
}

static_assert(align_nearest_down_pow2(4095, 4096) == 0);
static_assert(align_nearest_down_pow2(4096, 4096) == 4096);

static_assert(align_nearest_up_pow2(4095, 4096) == 4096);
static_assert(align_nearest_up_pow2(4096, 4096) == 4096);

template<class T, class... ArgTs>
constexpr T* construct_at(T* at, ArgTs&&... args) {
    return new ((void*)at) T(std::forward<ArgTs>(args)...);
}
} // namespace tos
