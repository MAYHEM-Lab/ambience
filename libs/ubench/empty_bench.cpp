#include <tos/debug/debug.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>

namespace {
void BM_EmptyBody(tos::bench::any_state& state) {
    for (auto _ : state) {
    }
}

BENCHMARK(BM_EmptyBody);
} // namespace