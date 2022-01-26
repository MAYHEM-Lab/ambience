#pragma once

namespace tos {
struct ignore_t {
    template<class... T>
    void operator()(T&&...) const {
    }
};

static constexpr ignore_t ignore{};
} // namespace tos