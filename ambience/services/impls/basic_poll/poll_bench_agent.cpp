#include "tos/task.hpp"
#include <agent_generated.hpp>
#include <cstdint>
#include <poll_generated.hpp>
#include <tos/ae/user_space.hpp>

namespace {
    struct async_poll_bench_agent : tos::ae::agent::async_server {
        async_poll_bench_agent(tos::ae::services::poll::async_server* p) : m_p{p} {}

        tos::Task<tos::ae::bench_result> start(const int64_t& num_iterations) override {
            auto i = 0;
            uint64_t begin, end = 0;
            begin = tos::ae::timestamp();
            // for (i = 0; i < num_iterations; i++) {
                co_await m_p->fn();
            // }
            end = tos::ae::timestamp();

            co_return tos::ae::bench_result{end - begin, 0, 0, 0};
        }
        tos::ae::services::poll::async_server* m_p;
    };
}

tos::Task<tos::ae::agent::async_server*>
init_async_poll_bench_agent(tos::ae::services::poll::async_server* p) {
    co_return new async_poll_bench_agent{p};
}