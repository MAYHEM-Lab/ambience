#include <agent_generated.hpp>
#include <posts_generated.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/debug/log.hpp>
#include <tos/detail/poll.hpp>

namespace social_media {
namespace {
struct posts_bench_agent : tos::ae::agent::async_server {
    posts_bench_agent(social_media::posts::async_server* posts_serv)
        : m_posts_serv{posts_serv} {
    }


    tos::Task<bool> start() override {
        constexpr auto extent = 100'000;
        constexpr auto concurrency = 10;
        tos::coro::countdown cd{concurrency};

        co_await cd.start([&] {
            for (int c = 0; c < concurrency; ++c) {
                tos::coro::make_detached([c, this, &cd]() -> tos::Task<void> {
                    for (int i = c * (extent / concurrency);
                         i < (c + 1) * (extent / concurrency);
                         ++i) {
                        auto id = co_await m_posts_serv->send_post("foobar", "Hello @bulut #meow!");
                    }
                    co_await cd.signal();
                });
            }
        });
        co_return true;
    }

    social_media::posts::async_server* m_posts_serv;
};
} // namespace
} // namespace social_media

tos::Task<tos::ae::agent::async_server*>
init_posts_bench_agent(social_media::posts::async_server* posts_serv) {
    co_return new social_media::posts_bench_agent(posts_serv);
}