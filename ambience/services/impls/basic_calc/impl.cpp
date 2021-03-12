#include <calc_generated.hpp>

namespace {
struct impl : tos::ae::service::calculator {
    int32_t add(const int32_t& x, const int32_t& y) override {
        return x + y;
    }
};
} // namespace

extern "C" tos::ae::service::calculator* init() {
    return new impl;
}