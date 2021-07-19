#include <agent_generated.hpp>
#include <calc_generated.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/detail/poll.hpp>

namespace {
struct async_calc_bench : tos::ae::agent::async_server {
    async_calc_bench(tos::ae::services::calculator::async_server* calc)
        : m_calc{calc} {
    }

    tos::Task<bool> start() override {

        constexpr auto extent = 1'000;
        constexpr auto concurrency = 10;
        tos::coro::countdown cd{.count = concurrency};
        for (int c = 0; c < concurrency; ++c) {
            tos::coro::make_detached([c, this, &cd]() -> tos::Task<void> {
                for (int i = c * (extent / concurrency);
                     i < (c + 1) * (extent / concurrency);
                     ++i) {
                    for (int j = 0; j < extent; ++j) {
                        co_await m_calc->add(i, j);
                    }
                }
                co_await cd.signal();
            });
        }

        co_await cd;
        co_return true;
    }

    tos::ae::services::calculator::async_server* m_calc;
};

struct calc_bench : tos::ae::agent::sync_server {
    calc_bench(tos::ae::services::calculator::sync_server* calc)
        : m_calc{calc} {
    }

    bool start() override {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                m_calc->add(i, j);
            }
        }
        return true;
    }

    tos::ae::services::calculator::sync_server* m_calc;
};
} // namespace

tos::ae::agent::sync_server*
init_calc_bench_agent(tos::ae::services::calculator::sync_server* calc) {
    return new calc_bench{calc};
}

tos::Task<tos::ae::agent::async_server*>
init_async_calc_bench_agent(tos::ae::services::calculator::async_server* calc) {
    co_return new async_calc_bench{calc};
}