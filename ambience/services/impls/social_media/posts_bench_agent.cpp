#include <agent_generated.hpp>
#include <posts_generated.hpp>

namespace social_media {
namespace {
struct posts_bench_agent : tos::ae::agent::async_server {

    tos::Task<bool> start() override {
//        co_await m_posts_serv->send_post("foobar", "Hello!");
        co_return true;
    }

//    social_media::posts::async_server* m_posts_serv;
};
} // namespace
} // namespace social_media

tos::Task<tos::ae::agent::async_server*>
init_posts_bench_agent(social_media::posts::async_server* posts_serv) {
    co_return new social_media::posts_bench_agent();
}