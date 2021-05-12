#include <calc_generated.hpp>
#include <tos/task.hpp>

namespace {
struct impl : tos::ae::services::calculator::async_server {
    tos::Task<int32_t> add(const int32_t& x, const int32_t& y) override {
        co_return x + y;
    }
};
} // namespace

tos::Task<tos::ae::services::calculator::async_server*> init_basic_calc() {
    co_return new impl;
}