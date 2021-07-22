#include <agent_generated.hpp>
#include <posts_generated.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/debug/log.hpp>
#include <tos/detail/poll.hpp>
#include <file_system_generated.hpp>
#include <tos/barrier.hpp>

namespace social_media {
namespace {
inline uint64_t rdtsc() {
    tos::detail::memory_barrier();
#if __has_builtin(__builtin_ia32_rdtsc)
    return __builtin_ia32_rdtsc();
#else
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
#endif
}
struct posts_bench_agent : tos::ae::agent::async_server {
    posts_bench_agent(social_media::posts::async_server* posts_serv, tos::ae::services::filesystem::async_server* fs)
        : m_posts_serv{posts_serv}, m_fs{fs} {
    }


    tos::Task<bool> start(const int64_t& param) override {
        tos::debug::log("Posts bench agent with param", param);
        constexpr auto extent = 1'000'000;
        const int concurrency = param;
        tos::coro::countdown cd{concurrency};

        std::vector<uint32_t> cycles(extent);

        co_await cd.start([&] {
            for (int c = 0; c < concurrency; ++c) {
                tos::coro::make_detached(
                    [c, this, &cd, concurrency, &cycles]() -> tos::Task<void> {
                        for (int i = c * (extent / concurrency);
                             i < (c + 1) * (extent / concurrency);
                             ++i) {
                            auto before = rdtsc();
                            auto id = co_await m_posts_serv->send_post(
                                "foobar", "Hello @bulut #meow!");
                            auto after = rdtsc();
                            auto diff_cycles = after - before;
                            cycles[i] = diff_cycles;
                        }
                        co_await cd.signal();
                    });
            }
        });

//        auto handle = co_await m_fs->open("/experiment.bin");
//
//        tos::debug::log("Write:", co_await m_fs->write_file(handle, 0, tos::raw_cast<uint8_t>(tos::span(cycles))));
//
//        tos::debug::log("Close:", co_await m_fs->close_file(handle));

        //        std::sort(cycles.begin(), cycles.end());
        std::nth_element(
            cycles.begin(), cycles.begin() + cycles.size() / 2, cycles.end());
        tos::debug::log("Median", *(cycles.begin() + cycles.size() / 2));
        std::nth_element(
            cycles.begin(), cycles.begin() + cycles.size() / 100 * 90, cycles.end());
        tos::debug::log("90th percentile", *(cycles.begin() + cycles.size() / 100 * 90));
        std::nth_element(
            cycles.begin(), cycles.begin() + cycles.size() / 100 * 99, cycles.end());
        tos::debug::log("99th percentile", *(cycles.begin() + cycles.size() / 100 * 99));
        //        tos::debug::log(extent/concurrency, "reqs in", total, "cycles");
        co_return true;
    }

    social_media::posts::async_server* m_posts_serv;
    tos::ae::services::filesystem::async_server* m_fs;
};
} // namespace
} // namespace social_media

tos::Task<tos::ae::agent::async_server*>
init_posts_bench_agent(social_media::posts::async_server* posts_serv, tos::ae::services::filesystem::async_server* fs) {
    co_return new social_media::posts_bench_agent(posts_serv, fs);
}