#include "lidlrt/builder.hpp"
#include "tos/coro/generate.hpp"
#include "tos/coro/transform.hpp"
#include "tos/coro/when_all.hpp"
#include <agent_generated.hpp>
#include <dns_generated.hpp>
#include <string_view>
#include <tos/ae/user_space.hpp>
#include <tos/debug/log.hpp>
#include <utility>

namespace tos::ae::services {
struct bench_agent : agent::async_server {
    bench_agent(tos::ae::services::dns::async_server* dns)
        : m_dns{dns} {
    }
    tos::Task<tos::ae::bench_result> start(const int64_t& param) override {
        std::vector<std::array<uint8_t, 32>> bufs(param);
        std::vector<lidl::message_builder> mb;
        mb.reserve(param);
        for (auto& buf : bufs) {
            mb.emplace_back(buf);
        }

        std::vector<std::string_view> results(param);
        auto now = tos::ae::timestamp();
        co_await tos::coro::generate_n(
            param, results.begin(), [&](int i) {
                return m_dns->resolve(hosts[(head + i) % hosts.size()], "", mb[i]);
            });
        head++;
        auto dur = tos::ae::timestamp() - now;
        tos::debug::log(dur);

        // auto now = tos::ae::timestamp();
        // auto results = co_await tos::coro::when_all(
        //     m_dns->resolve(hosts[(head + 0) % hosts.size()], "", mb[0]),
        //     m_dns->resolve(hosts[(head + 1) % hosts.size()], "", mb[1]),
        //     m_dns->resolve(hosts[(head + 2) % hosts.size()], "", mb[2]),
        //     m_dns->resolve(hosts[(head + 3) % hosts.size()], "", mb[3]),
        //     m_dns->resolve(hosts[(head + 4) % hosts.size()], "", mb[4]),
        //     m_dns->resolve(hosts[(head + 5) % hosts.size()], "", mb[5]),
        //     m_dns->resolve(hosts[(head + 6) % hosts.size()], "", mb[6]),
        //     m_dns->resolve(hosts[(head + 7) % hosts.size()], "", mb[7]),
        //     m_dns->resolve(hosts[(head + 8) % hosts.size()], "", mb[8]),
        //     m_dns->resolve(hosts[(head + 9) % hosts.size()], "", mb[9]));
        // head += 1;
        // auto dur = tos::ae::timestamp() - now;
        // [&]<size_t... Is>(std::index_sequence<Is...>) {
        //     tos::debug::log(std::get<Is>(results)..., dur);
        // }
        // (std::make_index_sequence<std::tuple_size_v<decltype(results)>>{});
        co_return tos::ae::bench_result{dur, 1, 2, 3};
    }

    static constexpr std::array<std::string_view, 16> hosts{
        "example.com",
        "a6e.org",
        "google.com",
        "lidl.dev",
        "invalid",
        "example.com",
        "a6e.org",
        "invalid",
        "google.com",
        "lidl.dev",
        "google.com",
        "lidl.dev",
        "invalid",
        "example.com",
        "a6e.org",
        "invalid",
    };
    int head = 0;
    tos::ae::services::dns::async_server* m_dns;
};
} // namespace tos::ae::services

tos::Task<tos::ae::agent::async_server*>
init_dns_bench_agent(tos::ae::services::dns::async_server* dns) {
    co_return new tos::ae::services::bench_agent{dns};
}