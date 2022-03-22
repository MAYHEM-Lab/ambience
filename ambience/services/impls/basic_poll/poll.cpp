#include "tos/task.hpp"
#include <cstdint>
#include <poll_generated.hpp>
#include <tos/ae/user_space.hpp>

namespace {
    struct basic_poll : tos::ae::services::poll::sync_server {
        uint64_t fn() override {
            return tos::ae::timestamp();;
        }
    };

    struct async_basic_poll : tos::ae::services::poll::async_server {
        tos::Task<uint64_t> fn() override {
            co_return tos::ae::timestamp();;
        }
    };
}

tos::ae::services::poll::sync_server* init_basic_poll() {
    return new basic_poll();
}

tos::Task<tos::ae::services::poll::async_server*> init_async_basic_poll() {
    co_return new async_basic_poll();
}