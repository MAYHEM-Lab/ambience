#include <agent_generated.hpp>
#include <calc_generated.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/debug/log.hpp>

namespace {
constexpr auto extent = 1'000;

struct async_calc_bench : tos::ae::agent::async_server {
    async_calc_bench(tos::ae::services::calculator::async_server* calc)
        : m_calc{calc} {
    }

    tos::Task<tos::ae::bench_result> start(const int64_t& concurrency) override {
        tos::coro::countdown cd{static_cast<int>(concurrency)};

        auto begin = tos::ae::timestamp();
        tos::debug::log(begin);
        co_await cd.start([&] {
            for (int c = 0; c < concurrency; ++c) {
                tos::coro::make_detached(
                    [c, this, &cd, concurrency]() -> tos::Task<void> {
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
        });
        auto end = tos::ae::timestamp();
        tos::debug::log(end, end-begin);
        co_return tos::ae::bench_result{end - begin, 0, 0, 0};
    }

    tos::ae::services::calculator::async_server* m_calc;
};

struct calc_bench : tos::ae::agent::sync_server {
    calc_bench(tos::ae::services::calculator::sync_server* calc)
        : m_calc{calc} {
    }

    tos::ae::bench_result start(const int64_t&) override {
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                m_calc->add(i, j);
            }
        }
        return {1, 0, 0, 0};
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