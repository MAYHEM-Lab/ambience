#include <agent_generated.hpp>
#ifdef __linux__
#include <chrono>
#else
#ifdef __x86_64__
#include <tos/x86_64/assembly.hpp>
#elif defined(__arm__)
#include <tos/arm/core.hpp>
#endif
#endif
namespace {
struct timestamper : tos::ae::nullaryfn::async_server {
    tos::Task<uint64_t> run() override {
#ifdef __linux__
        co_return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#else
#ifdef __x86_64__
        co_return tos::x86_64::rdtsc();
#elif defined __arm__
        co_return tos::arm::DWT::CYCCNT.read();
#endif
#endif
        co_return 0;
    }
};
} // namespace

tos::Task<tos::ae::nullaryfn::async_server*> init_timestamp() {
    co_return new timestamper;
}