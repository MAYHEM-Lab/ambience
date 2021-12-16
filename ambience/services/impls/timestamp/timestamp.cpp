#include <agent_generated.hpp>
#ifdef __x86_64__
#include <tos/x86_64/assembly.hpp>
#endif

namespace {
struct timestamper : tos::ae::nullaryfn::async_server {
    tos::Task<uint64_t> run() override {
#ifdef __x86_64__
        co_return tos::x86_64::rdtsc();
#endif
        co_return 0;
    }
};
} // namespace

tos::Task<tos::ae::nullaryfn::async_server*> init_timestamp() {
    co_return new timestamper;
}