#pragma once

#include <type_traits>

namespace caplets {
template<class T>
concept Frame = requires(const T& t) {
    t.capabilities();
    t.constraints();
};

template<class T>
concept Token = requires(const T& t) {
    t.tag();
    { t.frames().back() } -> Frame;
};

template<caplets::Frame FrameType>
struct frame_traits {
    using capabilities_type = std::remove_reference_t<
        decltype(std::declval<FrameType&>().capabilities().front())>;
    using constraints_type = std::remove_reference_t<
        decltype(std::declval<FrameType&>().constraints().front())>;
};

template<caplets::Token TokenType>
struct token_traits {
    using frame_type =
        std::remove_reference_t<decltype(std::declval<TokenType&>().frames().front())>;
};
} // namespace caplets