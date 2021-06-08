#include <tos/caplets/ebpf_constraint.hpp>
#include <tos/ebpf/vm.hpp>

namespace tos::ebpf {
bool verify(const eBPFConstraint& ctr) {
    uint8_t memory[256];

    execution_context ctx{
        .instructions = span<const instruction>(
            reinterpret_cast<const instruction*>(ctr.program().span().data()),
            ctr.program().size()),
        .memory = memory};

    return execute(ctx) == 1;
}
} // namespace tos::ebpf