#include <alarm_generated.hpp>
#include <calc_generated.hpp>
#include <file_system_generated.hpp>
#include <log_generated.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/task.hpp>

namespace {
struct impl : tos::ae::services::calculator::async_server {
    tos::Task<int32_t> add(const int32_t& x, const int32_t& y) override {
        co_return x + y;
    }
    tos::Task<float> mul(const float& x, const float& y) override {
        co_return x * y;
    }
    tos::Task<float> fma(const float& x, const float& y, const float& z) override {
        co_return x * y + z;
    }
};

struct sync_impl : tos::ae::services::calculator::sync_server {
    int32_t add(const int32_t& x, const int32_t& y) override {
        return x + y;
    }
    float mul(const float& x, const float& y) override {
        return x * y;
    }
    float fma(const float& x, const float& y, const float& z) override {
        return x * y + z;
    }
};
} // namespace

tos::Task<tos::ae::services::calculator::async_server*> init_basic_calc() {
    co_return new impl;
}

tos::ae::services::calculator::async_server* init_sync_basic_calc() {
    return new impl;
}