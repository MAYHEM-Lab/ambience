//
// Created by fatih on 8/22/18.
//

#pragma once
#include <tos/char_stream.hpp>

namespace tos
{
    namespace posix
    {
        struct file_desc
                : char_iostream
        {
        };

        template <class T>
        auto read_td(T& t, span<char> buf, std::true_type)
        { return t.read(buf); }

        template <class T>
        auto read_td(T&, span<char> buf, std::false_type)
        { return buf.slice(0, 0); }

        template <class T>
        auto write_td(T& t, span<const char> buf, std::true_type)
        { return t.write(buf); }

        template <class T>
        auto write_td(T&, span<const char>, std::false_type)
        { return 0; }

        template <class T>
        struct wrapper_desc
                : public file_desc
        {
            span<char> read(span<char> buf) override {
                return read_td(m_t, buf, typename driver_traits<T>::has_read{});
            }

            int write(span<const char> buf) override {
                return write_td(m_t, buf, typename driver_traits<T>::has_write{});
            }

            explicit wrapper_desc(T& t) : m_t{t} {}
        private:
            T& m_t;
        };
    }
}