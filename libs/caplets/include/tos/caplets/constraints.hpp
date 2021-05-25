#pragma once

namespace caplets {
template<class TokenType>
struct common_validation_context {
    const TokenType* token;
};

template<class TokenType, class... TransportData>
struct validation_context
    : common_validation_context<TokenType>
    , TransportData... {};

template<class Context>
constexpr bool check_constraints(const Context& ctx) {
    auto& tok = *ctx.token;
    auto& leaf_frame = tok.frames().back();

    for (auto& constr : leaf_frame.constraints()) {
        auto res = visit([&](auto& c) { return verify(c, ctx); }, constr);
        if (!res) {
            return false;
        }
    }

    return true;
}
} // namespace caplets