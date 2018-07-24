//
// Created by fatih on 5/3/18.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    template <class ErrT>
    struct unexpected_t {
        ErrT m_err;
    };

    template <class ErrT>
    auto unexpected(ErrT&& err)
    {
        return unexpected_t<ErrT>{ std::forward<ErrT>(err) };
    }

    template <class T, class ErrT>
    class expected
    {
    public:
        template <class U>
        expected(U&& u) : m_t{std::forward<U>(u)}, m_have{true} {}

        template <class ErrU>
        expected(unexpected_t<ErrU>&& u) : m_err{std::move(u.m_err)}, m_have{false} {}

        explicit operator bool() const { return m_have; }

        T& get() { return m_t; }
        const T& get() const { return m_t; }

        ErrT& error() { return m_err; }
        const ErrT& error() const { return m_err; }

    private:
        union {
            T m_t;
            ErrT m_err;
        };
        bool m_have;
    };

    struct ignore_t
    {
        template <class T>
        void operator()(T&&){};
    } ignore;

    template <class ExpectedT, class HandlerT, class ErrHandlerT = const ignore_t&>
    void with(ExpectedT&& expected, HandlerT&& have_handler, ErrHandlerT&& err_handler = ignore)
    {
        if (expected)
        {
            have_handler(expected.get());
        }
        else
        {
            err_handler(expected.error());
        }
    };
}