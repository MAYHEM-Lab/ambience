#pragma once

#include <caplets_generated.hpp>
#include <type_traits>

namespace caplets {
template<class Crypto,
         class Type,
         std::enable_if_t<std::is_base_of_v<lidl::struct_base<Type>, Type>>* = nullptr>
void compute_tag(const Type& tok, const Crypto& crypto);

template<class Crypto,
         class Type,
         std::enable_if_t<std::is_base_of_v<lidl::union_base<Type>, Type>>* = nullptr>
void compute_tag(const Type& tok, const Crypto& crypto);

template<class Crypto>
void compute_tag(const lidl::vector<uint8_t>& vec, const Crypto& crypto) {
    crypto.append(vec.span());
}

template<class Crypto>
void compute_tag(const uint64_t& val, const Crypto& crypto) {
    crypto.append(tos::raw_cast(tos::monospan(val)));
}

template<class Crypto, class Type, class... Members>
void compute_tag_detail(const Type& obj,
                        const Crypto& crypto,
                        const std::tuple<Members...>& members) {
    (compute_tag((obj.*std::get<Members>(members).const_function)(), crypto), ...);
}

template<class Crypto,
         class Type,
         std::enable_if_t<std::is_base_of_v<lidl::struct_base<Type>, Type>>*>
void compute_tag(const Type& tok, const Crypto& crypto) {
    using traits = lidl::struct_traits<Type>;
    compute_tag_detail(tok, crypto, traits::members);
}

template<class Crypto,
         class Type,
         std::enable_if_t<std::is_base_of_v<lidl::union_base<Type>, Type>>*>
void compute_tag(const Type& tok, const Crypto& crypto) {
    auto idx = static_cast<uint8_t>(tok.alternative());
    crypto.append(tos::monospan(idx));
    visit([&](auto& cap) { compute_tag(cap, crypto); }, tok);
}

template<class Frame, class Crypto>
void compute_frame_tag(const Frame& tok, const Crypto& crypto) {
    for (auto& frame : tok.capabilities()) {
        compute_tag(frame, crypto);
    }
    for (auto& frame : tok.constraints()) {
        compute_tag(frame, crypto);
    }
}

template<class Crypto, class Token>
auto compute_token_tag(const Token& tok, tos::span<const uint8_t> secret) {
    Crypto c{secret};
    for (auto& frame : tok.frames()) {
        compute_frame_tag(frame, c);
        c.reset(c.finish().tag());
    }
    return c.finish();
}
} // namespace caplets
