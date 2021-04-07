#include <calc_generated.hpp>
#include <tos/task.hpp>

namespace {
struct impl : tos::ae::service::calculator::sync_server {
    int32_t add(const int32_t& x, const int32_t& y) override {
        return x + y;
    }
};
} // namespace

tos::Task<tos::ae::service::calculator::sync_server*> init_basic_calc() {
    co_return new impl;
}