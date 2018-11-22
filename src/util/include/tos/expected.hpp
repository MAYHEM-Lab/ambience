//
// Created by fatih on 5/3/18.
//

#pragma once

#include <tos/utility.hpp>
#include <memory>
#include <type_traits>
#include <tos/compiler.hpp>
#include <tos/debug.hpp>
#include <new>
#include <nonstd/expected.hpp>

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
[[noreturn]] void tos_force_get_failed(void*);

namespace tos
{
    template <class ErrT>
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
    template <class ErrT>
    auto unexpected(ErrT&& err)
    {
        return unexpected_t<ErrT>{ std::forward<ErrT>(err) };
    }

    template <class T, class ErrT>
    class expected
    {
    public:
        template <typename = std::enable_if_t<std::is_same<T, void>{}>>
        expected() : m_internal{} {}

        template <class U, typename = std::enable_if_t<!std::is_same<U, expected>{}>>
        expected(U&& u) : m_internal{std::forward<U>(u)} {}

        template <class ErrU>
        expected(unexpected_t<ErrU>&& u) : m_internal{tl::make_unexpected(std::move(u.m_err))} {}

        constexpr explicit PURE operator bool() const { return bool(m_internal); }

        using value_type = T;
        using error_type = ErrT;
    private:

        std::enable_if_t<!std::is_same<T, void>{}, T&&>
        get() && { return std::move(*m_internal); }

        std::enable_if_t<!std::is_same<T, void>{}, T&>
        get() & { return *m_internal; }

        std::enable_if_t<!std::is_same<T, void>{}, const T&>
        get() const & { return *m_internal; }

        template <typename = std::enable_if_t<!std::is_same<ErrT, void>{}>>
        ErrT& error() { return m_internal.error(); }

        template <typename = std::enable_if_t<!std::is_same<ErrT, void>{}>>
        const ErrT& error() const { return m_internal.error(); }

        tl::expected<T, ErrT> m_internal;

        template <class HandlerT, class ErrHandlerT>
        friend auto ALWAYS_INLINE with(expected&& e, HandlerT&& have_handler, ErrHandlerT&& err_handler)
                -> decltype(have_handler(e.get()))
        {
            if (e)
            {
                return have_handler(e.get());
            }
            return err_handler(e.error());
        }

        friend auto ALWAYS_INLINE force_get(expected&& e) -> T
        {
            if (e)
            {
                return std::move(e.get());
            }

            tos_force_get_failed(nullptr);
        }

        friend auto ALWAYS_INLINE force_get(const expected& e) -> const T&
        {
            if (e)
            {
                return e.get();
            }

            tos_force_get_failed(nullptr);
        }

        friend auto ALWAYS_INLINE force_get(expected& e) -> T&
        {
            if (e)
            {
                return e.get();
            }

            tos_force_get_failed(nullptr);
        }
    };

    template <class T, class U>
    typename T::value_type get_or(T&& t, U&& r)
    {
        return with(std::forward<T>(t),
                [](auto&& res) -> decltype(auto) { return std::forward<decltype(res)>(res); },
                [&](auto&&) -> decltype(auto) { return r; });
    }

    struct ignore_t
    {
        template <class T>
        void operator()(T&&)const{}
    };
    static constexpr ignore_t ignore{};
}
