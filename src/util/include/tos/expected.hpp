//
// Created by fatih on 5/3/18.
//

#pragma once

#include <concepts>
#include <memory>
#include <new>
#include <nonstd/expected.hpp>
#include <optional>
#include <tos/compiler.hpp>
#include <tos/debug/assert.hpp>
#include <tos/error.hpp>
#include <tos/utility.hpp>
#include <type_traits>

/**
 * This function is called upon a force_get execution on an
 * expected object that's in unexpected state.
 *
 * This function is implemented as a weak symbol on supported
 * platforms and could be replaced by user provided
 * implementations.
 *
 * The implementation must never return to the caller, otherwise
 * the behaviour is undefined.
 */
[[noreturn]] NO_INLINE void tos_force_get_failed(void*);

namespace tos {
template<class ErrT>
struct unexpected_t {
    ErrT m_err;
};

/**
 * This function ttemplate is used to disambiguate an error from a success value
 * when the type of the error and the expected is the same:
 *
 *      expected<int, int> find(...)
 *      {
 *          if (found)
 *          {
 *              return index;
 *          }
 *          // return -1; -> would signal success
 *          return unexpected(-1); // signals error
 *      }
 *
 * @param err the error object
 * @tparam ErrT type of the error object
 */
template<class ErrT>
auto unexpected(ErrT&& err) {
    return unexpected_t<ErrT>{std::forward<ErrT>(err)};
}

template<class T, class ErrT>
class expected;

template<class T>
struct is_expected : std::false_type {};

template<class U, class V>
struct is_expected<expected<U, V>> : std::true_type {};

struct default_construct_tag_t {};
static constexpr default_construct_tag_t default_construct{};

template<class T, class ErrT>
class expected {
    using internal_t = tl::expected<T, ErrT>;

public:
    template<class U = T, typename = std::enable_if_t<std::is_same<U, void>{}>>
    constexpr expected()
        : m_internal{} {
    }

    template<class U = T, typename = std::enable_if_t<!std::is_same<U, void>{}>>
    constexpr expected(default_construct_tag_t)
        : m_internal{} {
    }

    template<class ErrU>
    constexpr expected(unexpected_t<ErrU>&& u)
        : m_internal{tl::make_unexpected(std::move(u.m_err))} {
    }

    template<Error Err>
    requires std::convertible_to<Err, ErrT>
    constexpr expected(Err&& err)
        : expected(unexpected(std::forward<Err>(err))) {
    }

    template<class U = T,
             typename = std::enable_if_t<!is_expected<std::remove_reference_t<U>>{}>>
    requires(!Error<U>) constexpr expected(U&& u)
        : m_internal(std::forward<U>(u)) {
    }

    template<class OtherT, class OtherErr>
    requires std::convertible_to<OtherT, T> && std::convertible_to<OtherErr, ErrT>
    constexpr expected(expected<OtherT, OtherErr>&& other)
        : m_internal(other.m_internal) {
    }

    PURE constexpr explicit operator bool() const {
        return bool(m_internal);
    }

    constexpr bool operator!() {
        return !bool(m_internal);
    }

    PURE constexpr bool has_value() const {
        return m_internal.has_value();
    }

    explicit constexpr operator std::optional<T>() const {
        if (!*this) {
            return std::nullopt;
        }

        return force_get(*this);
    }

    template<class U = T, std::enable_if_t<!std::is_same_v<void, U>>* = nullptr>
    decltype(auto) get_or(U&& t) && {
        return std::move(m_internal).value_or(std::move(t));
    }

    using value_type = typename internal_t::value_type;
    using error_type = typename internal_t::error_type;

private:
    internal_t m_internal;

    template<class ExpectedT, class HandlerT, class ErrHandlerT>
    friend auto with(ExpectedT&& e, HandlerT&& have_handler, ErrHandlerT&&)
        -> decltype(have_handler(std::forward<decltype(*e.m_internal)>(*e.m_internal)));

    template<class ExpectedT,
             class UnexpectedT,
             std::enable_if_t<!std::is_same_v<ExpectedT, void>>*>
    friend ExpectedT& force_get(expected<ExpectedT, UnexpectedT>& e);

    template<class ExpectedT,
             class UnexpectedT,
             std::enable_if_t<!std::is_same_v<ExpectedT, void>>*>
    friend ExpectedT&& force_get(expected<ExpectedT, UnexpectedT>&& e);

    template<class UnexpectedT>
    friend void force_get(expected<void, UnexpectedT>& e);

    template<class ExpectedT>
    friend decltype(auto) force_error(ExpectedT&&);
};

template<class ExpectedT, class HandlerT, class ErrHandlerT>
ALWAYS_INLINE auto with(ExpectedT&& e, HandlerT&& have_handler, ErrHandlerT&& err_handler)
    -> decltype(have_handler(std::forward<decltype(*e.m_internal)>(*e.m_internal))) {
    if (e) {
        return have_handler(std::forward<decltype(*e.m_internal)>(*e.m_internal));
    }
    return err_handler(
        std::forward<decltype(e.m_internal.error())>(e.m_internal.error()));
}

template<class ExpectedT>
ALWAYS_INLINE decltype(auto) force_error(ExpectedT&& e) {
    if (!e) {
        return e.m_internal.error();
    }

    tos_force_get_failed(nullptr);
}

template<class UnexpectedT>
ALWAYS_INLINE void force_get(expected<void, UnexpectedT>& e) {
    if (!e) {
        tos_force_get_failed(nullptr);
    }
}

template<class ExpectedT,
         class UnexpectedT,
         std::enable_if_t<!std::is_same_v<ExpectedT, void>>* = nullptr>
ALWAYS_INLINE ExpectedT&& force_get(expected<ExpectedT, UnexpectedT>&& e) {
    if (e) {
        return std::move(*e.m_internal);
    }

    tos_force_get_failed(nullptr);
}

template<class ExpectedT,
         class UnexpectedT,
         std::enable_if_t<!std::is_same_v<ExpectedT, void>>* = nullptr>
ALWAYS_INLINE ExpectedT& force_get(expected<ExpectedT, UnexpectedT>& e) {
    if (e) {
        return *e.m_internal;
    }

    tos_force_get_failed(nullptr);
}

template<class T, class U>
typename T::value_type get_or(T&& t, U&& r) {
    return with(
        std::forward<T>(t),
        [](auto&& res) -> decltype(auto) { return std::forward<decltype(res)>(res); },
        [&](auto&&) -> decltype(auto) { return r; });
}

struct ignore_t {
    template<class... T>
    void operator()(T&&...) const {
    }
};
static constexpr ignore_t ignore{};

template<class T, class E>
void ensure(const expected<T, E>& expect) {
    Assert(expect.has_value());
}
} // namespace tos

#define EXPECTED_TRYV(...)                                \
    __extension__({                                       \
        auto&& __res = (__VA_ARGS__);                     \
        if (!__res)                                       \
            return ::tos::unexpected(force_error(__res)); \
    })

#define EXPECTED_TRY(...)                                 \
    __extension__({                                       \
        auto&& __res = (__VA_ARGS__);                     \
        if (!__res)                                       \
            return ::tos::unexpected(force_error(__res)); \
        std::move(force_get(__res));                      \
    })

#if !defined(TOS_CONFIG_DISABLE_SHORT_TRY)
#define TRY  EXPECTED_TRY
#define TRYV EXPECTED_TRYV
#endif