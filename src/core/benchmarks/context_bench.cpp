#include <tos/context.hpp>
#include <tos/debug/debug.hpp>
#include <tos/ft.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>

namespace tos {
namespace {
void BM_Context_Get_Existing_Component(bench::any_state& state) {
    for (auto _ : state) {
        auto component = current_context().get_component_with_id(1);
        tos::debug::do_not_optimize(&component);
    }
}

BENCHMARK(BM_Context_Get_Existing_Component);

void BM_Context_Get_Missing_Component(bench::any_state& state) {
    for (auto _ : state) {
        auto component = current_context().get_component_with_id(0);
        tos::debug::do_not_optimize(&component);
    }
}

BENCHMARK(BM_Context_Get_Missing_Component);
} // namespace
} // namespace tos