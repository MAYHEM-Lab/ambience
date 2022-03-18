#include "tos/task.hpp"
#include <poll_generated.hpp>

namespace {
    struct async_basic_poll : tos::ae::services::poll::async_server {
        tos::Task<bool> fn() override {
            co_return true;
        }
    };
}

tos::Task<tos::ae::services::poll::async_server*> init_async_basic_poll() {
    co_return new async_basic_poll();
}