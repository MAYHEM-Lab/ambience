#include "tos/task.hpp"
#include <agent_generated.hpp>
#include <cstdint>
#include <poll_generated.hpp>
#include <tos/ae/user_space.hpp>

namespace {
    struct async_poll_bench_agent : tos::ae::agent::async_server {
        async_poll_bench_agent(tos::ae::services::poll::async_server* p) : m_p{p} {}

        tos::Task<tos::ae::bench_result> start(const int64_t& num_iterations) override {
            tos::debug::log("Poll bench agent, num_iterations:", num_iterations);
            
            auto i = 0;
            uint64_t begin = 0;
            uint64_t end = 0;
            uint64_t before = 0;
            uint64_t after = 0;
            uint64_t stamp = 0;

            // We want to calculate how long it takes to call and return from a service
            uint64_t call_time = 0;
            uint64_t call_sum = 0;
            uint64_t call_sq_sum = 0;
            uint64_t return_time = 0;
            uint64_t return_sum = 0;
            uint64_t return_sq_sum = 0;

            begin = tos::ae::timestamp();
            for (i = 0; i < num_iterations; i++) {
                // Call another service that simply returns a timestamp
                before = tos::ae::timestamp();
                stamp = co_await m_p->fn();
                after = tos::ae::timestamp();
                
                // Update the call time variables
                call_time = stamp - before;
                call_sum += call_time;
                call_sq_sum += call_time * call_time;

                // Update the return time variables
                return_time = after - stamp;
                return_sum += return_time;
                return_sq_sum += return_time * return_time;
            }
            end = tos::ae::timestamp();
            auto total = end - begin;
            tos::debug::log(total, call_sum, call_sq_sum, return_sum, return_sq_sum);

            co_return tos::ae::bench_result{call_sum, call_sq_sum, return_sum, return_sq_sum};
        }
        tos::ae::services::poll::async_server* m_p;
    };
}

tos::Task<tos::ae::agent::async_server*>
init_async_poll_bench_agent(tos::ae::services::poll::async_server* p) {
    co_return new async_poll_bench_agent{p};
}