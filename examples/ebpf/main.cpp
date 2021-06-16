#include <lidlrt/builder.hpp>
#include <policy_generated.hpp>
#include <tos/board.hpp>
#include <tos/ebpf/vm.hpp>
#include <tos/ft.hpp>
#include <tos/hex.hpp>

void wasm_task() {
    auto com = tos::bsp::board_spec::default_com::open();

    tos::println(com, "Hello");

    constexpr auto bytes = tos::hex_to_bytes(
        "b702000000000000732afaff00000000b70200006f7700006b2af8ff00000000180200002f686f6d"
        "00000000652f6d657b2af0ff0000000069130000000000006703000030000000c703000030000000"
        "bf140000000000001f3400000000000069420000000000006702000030000000c702000030000000"
        "b70500000a0000002d25010000000000b70200000a0000001502180000000000b700000000000000"
        "714402000000000071a5f0ff000000005d541900000000008703000000000000bf14000000000000"
        "0f34000000000000b703000000000000bfa500000000000007050000f1ffffffbf20000000000000"
        "07000000ffffffff07040000030000001d300a0000000000bf460000000000000f36000000000000"
        "bf570000000000000f37000000000000070300000100000071770000000000007166000000000000"
        "1d76f7ff00000000b7000000000000002d3205000000000071110200000000005701000002000000"
        "b7000000010000001501010000000000b7000000000000005700000001000000950000000000000"
        "0");

    uint8_t memory[256];

    tos::ebpf::execution_context ctx{
        .instructions = tos::span<const tos::ebpf::instruction>(
            reinterpret_cast<const tos::ebpf::instruction*>(bytes.data()),
            bytes.size() / 8),
        .memory = memory};

    tos::span<uint8_t> mem_span(memory);
    lidl::message_builder builder(mem_span);

    auto& cap = lidl::create<tos::path_capability>(
        builder, lidl::create_string(builder, "/home/meow/foo"), tos::permissions2::read);

    std::array<uint64_t, 1> args{reinterpret_cast<uint64_t>(&cap)};
    bool result = execute(ctx, args);

    tos::println(com, "Run result:", result);
}

void tos_main() {
    tos::launch(tos::alloc_stack, wasm_task);
}