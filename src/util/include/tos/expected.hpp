//
// Created by fatih on 5/3/18.
//

#pragma once

#include <tos/utility.hpp>
#include <tos/memory.hpp>
#include <tos/type_traits.hpp>
#include <tos/compiler.hpp>
#include <tos/new.hpp>

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
        expected(const expected&) = delete;

        expected(expected<T, ErrT>&& rhs) noexcept
            : m_have(rhs.m_have)
        {
            if (m_have)
            {
                new (&m_t) T(tos::std::move(rhs.m_t));
            }
            else
            {
                new (&m_err) ErrT(tos::std::move(rhs.m_err));
            }
            rhs.m_have = false;
        }

        template <class U, typename = tos::std::enable_if_t<!tos::std::is_same<U, expected>{}>>
        expected(U&& u) : m_t{std::forward<U>(u)}, m_have{true} {}

        template <class ErrU>
        expected(unexpected_t<ErrU>&& u) : m_err{std::move(u.m_err)}, m_have{false} {}

        explicit operator bool() const { return m_have; }

        expected& operator=(expected&& rhs) noexcept
        {
            if (m_have && rhs.m_have)
            {
                m_t = std::move(rhs.m_t);
            }
            else if (!m_have && !rhs.m_have)
            {
                m_err = std::move(rhs.m_err);
            }
            else if (m_have && !rhs.m_have)
            {
                std::destroy_at(&m_t);
                new (&m_err) ErrT(std::move(rhs.m_err));
            }
            else // if (!m_have && rhs.m_have)
            {
                std::destroy_at(&m_err);
                new (&m_t) T(std::move(rhs.m_t));
            }
            m_have = rhs.m_have;
            rhs.m_have = false;
            return *this;
        }

        ~expected()
        {
            if (m_have)
            {
                std::destroy_at(&m_t);
            }
            else
            {
                std::destroy_at(&m_err);
            }
        }
    private:
        T& get() { return m_t; }
        const T& get() const { return m_t; }

        ErrT& error() { return m_err; }
        const ErrT& error() const { return m_err; }

        union {
            char ___;
            T m_t;
            ErrT m_err;
        };

        bool m_have;

        template <class HandlerT, class ErrHandlerT>
        friend auto ALWAYS_INLINE with(expected&& e, HandlerT&& have_handler, ErrHandlerT&& err_handler)
                -> decltype(have_handler(e.get()))
        {
            if (e)
            {
                return have_handler(e.get());
            }
            return err_handler(e.error());
        };
    };

    struct ignore_t
    {
        template <class T>
        void operator()(T&&)const{};
    };
    static constexpr ignore_t ignore{};
}