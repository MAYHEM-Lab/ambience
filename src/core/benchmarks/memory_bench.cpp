#include <tos/allocator/free_list.hpp>
#include <tos/debug/debug.hpp>
#include <tos/ft.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>

namespace tos {
namespace {
void BM_FreeList_Allocate(bench::any_state& state) {
    alignas(16) static uint8_t buf[16*1024];
    memory::free_list alloc(buf);
    for (auto _ : state) {
        alloc.allocate(4);
    }
}

BENCHMARK(BM_FreeList_Allocate);
} // namespace
} // namespace tos