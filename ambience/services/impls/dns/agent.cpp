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
alignas(4096) static std::array<std::array<uint8_t, 64>, 1024 * 10> globbufs;
struct bench_agent : agent::async_server {
    bench_agent(tos::ae::services::dns::async_server* dns)
        : m_dns{dns} {
    }
    tos::Task<tos::ae::bench_result> start(const int64_t& param) override {
        tos::debug::log("agent", param);

        std::vector<lidl::message_builder> mb(param);
        std::vector<std::string_view> results(param);
        uint64_t latency_sum = 0;
        uint64_t latency_sq = 0;
        auto now = tos::ae::timestamp();
        for (auto j = 0; j < 1024 * 10; j += param) {
            auto bufs = tos::span(globbufs).slice(j, param);
            std::transform(bufs.begin(), bufs.end(), mb.begin(), [](auto& arr) {
                return lidl::message_builder(arr);
            });

            auto now = tos::ae::timestamp();
            co_await tos::coro::generate_n(param, results.begin(), [&](int i) {
                return m_dns->resolve(hosts[(head + i) % hosts.size()], "", mb[i]);
            });
            auto end = tos::ae::timestamp();
            auto diff = end - now;
            latency_sum += diff;
            latency_sq += diff * diff;
            head++;
        }
        auto dur = tos::ae::timestamp() - now;
        tos::debug::log(dur, latency_sum, latency_sq);

        // auto now = tos::ae::timestamp();
        // auto results = co_await tos::coro::when_all(
        //     m_dns->resolve(hosts[(head + 0) % hosts.size()], "", mb[0]),
        //     m_dns->resolve(hosts[(head + 1) % hosts.size()], "", mb[1]));
        // head += 1;
        // auto dur = tos::ae::timestamp() - now;
        // [&]<size_t... Is>(std::index_sequence<Is...>) {
        //     tos::debug::log(std::get<Is>(results)..., dur);
        // }
        // (std::make_index_sequence<std::tuple_size_v<decltype(results)>>{});
        co_return tos::ae::bench_result{dur, latency_sum, latency_sq, tos::ae::default_allocator().in_use().value()};
    }

    static constexpr std::array<std::string_view, 16> hosts{
        "example.com",
        "invalid",
        "google.com",
        "lidl.dev",
        "invalid",
        "invalid",
        "a6e.org",
        "invalid",
        "google.com",
        "invalid",
        "google.com",
        "lidl.dev",
        "invalid",
        "invalid",
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