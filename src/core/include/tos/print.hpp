//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#pragma once

#include <chrono>
#include <cstddef>
#include <stdint.h>
#include <string.h>
#include <tos/span.hpp>
#include <tos/utility.hpp>

namespace tos {

namespace detail {
static constexpr char lookup[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
}

inline tos::span<const char> itoa(int64_t i, int base = 10) {
    static char intbuf[std::numeric_limits<decltype(i)>::digits10 + 1];

    int64_t j = 0, isneg = 0;

    if (i == 0) {
        return tos::span<const char>("0").slice(0, 1);
    }

    if (i < 0) {
        isneg = 1;
        i = -i;
    }

    while (i != 0) {
        intbuf[j++] = detail::lookup[(i % base)];
        i /= base;
    }

    if (isneg)
        intbuf[j++] = '-';

    auto len = j;
    intbuf[j] = '\0';
    j--;
    i = 0;
    while (i < j) {
        isneg = intbuf[i];
        intbuf[i] = intbuf[j];
        intbuf[j] = isneg;
        i++;
        j--;
    }

    return {intbuf, size_t(len)};
}
} // namespace tos

namespace tos {
template<class CharOstreamT>
void print(CharOstreamT& ostr, char c) {
    print(ostr, uint8_t(c));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, uint8_t b) {
    ostr->write(tos::span<const uint8_t>{&b, 1});
}

/**
 * Prints the given null terminated string to the output stream
 * @param ostr The output stream
 * @param str A null terminated string
 */
template<class CharOstreamT>
void print(CharOstreamT& ostr, const char* str) {
    auto len = strlen(str);
    if (len == 0)
        return;
    ostr->write(tos::span<const uint8_t>{reinterpret_cast<const uint8_t*>(str), len});
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, span<const char> buf) {
    if (buf.empty()) {
        return;
    }
    ostr->write(tos::raw_cast<const uint8_t>(buf));
}

template<class CharOstreamT,
         class T = int16_t,
         typename = std::enable_if_t<!std::is_same<T, int>{}>>
void print(CharOstreamT& ostr, int16_t x) {
    print(ostr, itoa(x, 10));
}

template<class CharOstreamT,
         class T = int32_t,
         typename = std::enable_if_t<!std::is_same<T, int>{}>>
void print(CharOstreamT& ostr, int32_t x) {
    print(ostr, itoa(x, 10));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, int x) {
    print(ostr, itoa(x, 10));
}


template<class CharOstreamT>
void print(CharOstreamT& ostr, double x) {
    static constexpr size_t decimal_places = 8;
    print(ostr, itoa((int)x));
    print(ostr, ".");
    for (size_t i = 0; i < decimal_places; ++i) {
        x *= 10.0;
        print(ostr, detail::lookup[((int)x) % 10]);
    }
}


template<class CharOstreamT,
         class U = uintptr_t,
         class = std::enable_if_t<!std::is_same_v<U, uint64_t>>>
void print(CharOstreamT& ostr, uintptr_t ptr) {
    // print(ostr, '0');
    // print(ostr, 'x');
    print(ostr, itoa(ptr, 16));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, void* p) {
    print(ostr, reinterpret_cast<uintptr_t>(p));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, uint32_t p) {
    print(ostr, itoa(p, 10));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, uint64_t p) {
    print(ostr, itoa(p, 10));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, int64_t p) {
    print(ostr, itoa(p, 10));
}

template<class CharOstreamT>
void print(CharOstreamT& ostr, bool b) {
    print(ostr, b ? "true" : "false");
}

template<class StrT>
void print(StrT& ostr, span<const uint8_t> buf) {
    for (auto byte : buf) {
        auto itoa_res = itoa(byte, 16);
        if (itoa_res.size() != 2) {
            print(ostr, '0');
        }
        print(ostr, itoa_res);
    }
}

namespace detail {
template<class T>
struct separator_t {
    T sep;

    template<class StreamT>
    friend void print(StreamT& str, const separator_t<T>& sep) {
        using tos::print;
        print(str, sep.sep);
    }
};

template<>
struct separator_t<std::nullptr_t> {
    template<class StreamT>
    friend void print(StreamT&, const separator_t<std::nullptr_t>&) {
    }
};

constexpr void get_separator_or() = delete;

template<class FirstT, class... Ts>
constexpr const auto& get_separator_or(const FirstT&, const Ts&... ts) {
    return get_separator_or(ts...);
}

template<class FirstT, class... Ts>
constexpr const separator_t<FirstT>& get_separator_or(const separator_t<FirstT>& first,
                                                      const Ts&...) {
    return first;
}
} // namespace detail
constexpr detail::separator_t<std::nullptr_t> no_separator() {
    return {};
}

template<class T>
constexpr detail::separator_t<T> separator(T&& sep) {
    return {std::forward<T>(sep)};
}

namespace detail {
template<class OstreamT, class T1, class T2>
void print2(OstreamT&, T1&&, separator_t<T2>&&) {
}

template<class OstreamT, class T1, class T2>
void print2(OstreamT& ostr, T1&& t1, T2&& t2) {
    print(ostr, std::forward<T1>(t1));
    print(ostr, std::forward<T2>(t2));
}
} // namespace detail

/**
 * Prints the given arguments with the given separator in between
 *
 * To prevent overload ambiguities, this function takes 2 regular arguments
 * and then a variadic pack of arguments
 *
 * @tparam T1 Type of the first parameter
 * @tparam T2 Type of the second parameter
 * @tparam Ts Types of the rest of the parameters
 * @param ostr Stream to print the output to
 * @param t1 First parameter
 * @param t2 Second parameter
 * @param ts Rest of the parameters
 * @param sep Separator
 */
template<class CharOstreamT, class T1, class T2, class... Ts>
void print(CharOstreamT& ostr, T1&& t1, T2&& t2, Ts&&... ts) {
    constexpr auto default_sep = separator(' ');
    auto sep = detail::get_separator_or(ts..., default_sep);
    print(ostr, std::forward<T1>(t1));
    detail::print2(ostr, sep, std::forward<T2>(t2));

    int _[] = {0, (detail::print2(ostr, sep, std::forward<Ts>(ts)), 0)...};
    (void)_;
}

template<class CharOstreamT>
void println(CharOstreamT& ostr) {
    print(ostr, "\r\n");
}

template<class CharOstreamT, class... T>
void println(CharOstreamT& ostr, T&&... t) {
    print(ostr, std::forward<T>(t)...);
    println(ostr);
}

template<class StrT>
void print(StrT& ostr, std::chrono::milliseconds ms) {
    print(ostr, int(ms.count()), "ms", tos::no_separator());
}

template<class StrT>
void print(StrT& ostr, std::chrono::microseconds us) {
    print(ostr, int(us.count()), "us", tos::no_separator());
}
} // namespace tos
