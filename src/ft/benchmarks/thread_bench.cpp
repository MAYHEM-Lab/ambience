#include <tos/debug/debug.hpp>
#include <tos/ft.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>
#include <tos/semaphore.hpp>

namespace tos {
namespace {
void BM_Yield(bench::any_state& state) {
    for (auto _ : state) {
        tos::this_thread::yield();
    }
}
BENCHMARK(BM_Yield);

void BM_Semaphore_Down(bench::any_state& state) {
    tos::semaphore_base<int32_t> sem{1'000'000};
    for (auto _ : state) {
        sem.down();
    }
}
BENCHMARK(BM_Semaphore_Down);
} // namespace
} // namespace tos