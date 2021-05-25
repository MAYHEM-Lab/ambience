#pragma once

#include <algorithm>
#include <concepts>
#include <tos/expected.hpp>

namespace caplets {
template<class L, class R>
concept CanImply = requires(const L& l, const R& r) {
    { implies(l, r) } -> std::same_as<bool>;
};

namespace detail {
template<class L, class R>
inline bool implies_detail(const L&, const R&) {
    return false;
}

template<class L, class R>
requires CanImply<L, R> bool implies_detail(const L& l, const R& r) {
    return implies(l, r);
}

template<class Ts, class Us>
bool implies(const Ts& l, const Us& r) {
    return visit(
        [&r](const auto& prev_cap) {
            return visit(
                [&prev_cap](const auto& next_cap) {
                    return implies_detail(prev_cap, next_cap);
                },
                r);
        },
        l);
}
} // namespace detail

enum class validation_error
{
    excess_capability,
    missing_constraint,
    missing_implies
};

template<class FrameT>
tos::expected<void, validation_error> validate_link(const FrameT& prev,
                                                    const FrameT& next) {
    for (auto& next_capability : next.capabilities()) {
        auto anything_implies =
            std::any_of(prev.capabilities().begin(),
                        prev.capabilities().end(),
                        [&](auto& prev_capability) {
                            using detail::implies;
                            return implies(prev_capability, next_capability);
                        });
        if (anything_implies) {
            continue;
        }
        return tos::unexpected(validation_error::excess_capability);
    }

    for (auto& prev_constraint : prev.constraints()) {
        auto it = std::find(
            next.constraints().begin(), next.constraints().end(), prev_constraint);
        if (it == next.constraints().end()) {
            return tos::unexpected(validation_error::missing_constraint);
        }
    }

    return {};
}

template<class FrameIteratorT>
tos::expected<void, validation_error> validate_chain(FrameIteratorT begin,
                                                     FrameIteratorT end) {
    auto next = begin;
    ++next;
    while (next != end) {
        if (auto res = validate_link(*begin, *next); !res) {
            return res;
        }
        ++begin;
        ++next;
    }
    return {};
}

template<class Token>
tos::expected<void, validation_error> validate_token(const Token& tok) {
    return validate_chain(tok.frames().begin(), tok.frames().end());
}
} // namespace caplets