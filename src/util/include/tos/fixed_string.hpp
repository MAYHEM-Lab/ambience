#pragma once

#include <algorithm>
#include <compare>
#include <string_view>

namespace tos {
template<size_t N>
struct fixed_string {
    constexpr fixed_string() = default;

    constexpr fixed_string(const char (&str)[N]) {
        std::copy(std::begin(str), std::end(str), std::begin(val));
    }

    char val[N] = {};

    constexpr auto size() const {
        return N;
    }

    constexpr auto begin() const {
        return std::begin(val);
    }

    constexpr auto begin() {
        return std::begin(val);
    }

    template<size_t M>
    constexpr bool operator==(const fixed_string<M>& rhs) const {
        return N == M && std::equal(std::begin(val), std::end(val), std::begin(rhs.val));
    }

    constexpr operator std::string_view() const {
        return std::string_view(val, N - 1);
    }

    template<size_t M>
    friend constexpr fixed_string<N + M - 1> operator+(const fixed_string<N>& l,
                                                   const fixed_string<M>& r) {
        fixed_string<N + M - 1> res{};
        auto end = std::copy(std::begin(l.val), std::end(l.val) - 1, std::begin(res.val));
        std::copy(std::begin(r.val), std::end(r.val), end);
        return res;
    }

    friend auto operator<=>(const fixed_string&, const fixed_string&) = default;
};

template<size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;
} // namespace tos
