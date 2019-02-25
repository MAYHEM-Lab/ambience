//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#pragma once

#include <stddef.h>
#include <tos/span.hpp>
#include "driver_traits.hpp"
#include <type_traits>
#include <common/driver_base.hpp>

namespace tos
{
    struct char_istream : self_pointing<char_istream>
    {
        virtual span<char> read(span<char>) = 0;
        virtual ~char_istream() = default;
    };

    struct char_ostream : self_pointing<char_ostream>
    {
        virtual int write(span<const char>) = 0;
        virtual ~char_ostream() = default;
    };

    struct char_iostream :
            public char_istream,
            public char_ostream
    {
    };

    template <class T>
    struct istream_facade : char_istream, self_pointing<istream_facade<T>>
    {
        template <class U = T>
        explicit istream_facade(U t) : m_t{std::forward<U>(t)} {}

        span<char> read(span<char> buf) override
        {
            return m_t.read(buf);
        }

    private:
        T m_t;
    };

    template <class T>
    struct ostream_facade : char_ostream, self_pointing<ostream_facade<T>>
    {
        template <class U = T>
        explicit ostream_facade(U&& t) : m_t{std::forward<U>(t)} {}

        int write(span<const char> buf) override
        {
            return m_t->write(buf);
        }

    private:
        T m_t;
    };

    template <class T>
    struct iostream_facade : char_iostream, self_pointing<iostream_facade<T>> {
        template <class U = T>
        explicit iostream_facade(U&& t) : m_t{std::forward<U>(t)} {}

        span<char> read(span<char> buf) override
        {
            return m_t.read(buf);
        }

        int write(span<const char> buf) override
        {
            return m_t.write(buf);
        }
    private:
        T m_t;
    };

    template <class T>
    using correct_base_t = std::conditional_t<
            has_read_v<T> && has_write_v<T>,
            iostream_facade<T>,
            std::conditional_t<has_read_v<T>,
                    istream_facade<T>,
                    std::conditional_t<has_write_v<T>,
                            ostream_facade<T>,
                            void>>>;

    template <class T>
    class stream_facade : public correct_base_t<T>
    {
    public:
        using correct_base_t<T>::correct_base_t;
    };
}
