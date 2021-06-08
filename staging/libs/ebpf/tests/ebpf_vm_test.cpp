#include <doctest.h>
#include <tos/ebpf/vm.hpp>

namespace tos::ebpf {
TEST_CASE("eBPF VM can run a simple program") {
    constexpr uint8_t prog[] = {0xb7,
                                0x00,
                                0x00,
                                0x00,
                                0x2a,
                                0x00,
                                0x00,
                                0x00,
                                0x95,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00};

    uint8_t memory[256];

    execution_context ctx{
        .instructions = span<const instruction>(
            reinterpret_cast<const instruction*>(prog), sizeof prog / 8),
        .memory = memory};

    REQUIRE_EQ(42, execute(ctx));
}
} // namespace tos::ebpf