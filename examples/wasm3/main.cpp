#include <lidlrt/builder.hpp>
#include <policy_generated.hpp>
#include <tos/board.hpp>
#include <tos/ft.hpp>
#include <tos/hex.hpp>
#include <wasm3.h>

void wasm_task() {
    auto com = tos::bsp::board_spec::default_com::open();

    tos::println(com, "Hello");

    auto env = m3_NewEnvironment();

    auto rt = m3_NewRuntime(env, 1024, nullptr);

    constexpr auto bytes = tos::hex_to_bytes(
        "0061736d0100000001060160017f017f0302010005030100020608017f01419088040b071002066d"
        "656d6f727902000372756e00000ac40101c10101077f4100210123808080800041106b2202410028"
        "008788808000360007200241002900808880800037030002400240200020002e010022036b22042e"
        "01002205410a2005410a491b2206450d0020042d000220022d0000470d012006417f6a2104200041"
        "0020036b6a41036a21032002410172210741002102034020042002460d01200720026a2101200320"
        "026a2105200241016a210220052d000020012d0000460d000b4100210120022006490d010b20002d"
        "00024102714521010b20010b0b1201004180080b0b2f686f6d652f6d656f7700");

    IM3Module mod;
    if (m3_ParseModule(env, &mod, bytes.data(), bytes.size()) != m3Err_none) {
        tos::println(com, "Error parsing");
    }

    if (m3_LoadModule(rt, mod) != m3Err_none) {
        tos::println(com, "Error loading");
    }

    IM3Function run;

    if (m3_FindFunction(&run, rt, "run") != m3Err_none) {
        tos::println(com, "Error finding");
    }

    uint32_t mem_sz;
    auto mem = m3_GetMemory(rt, &mem_sz, 0);

    tos::span<uint8_t> mem_span(mem, ptrdiff_t(mem_sz));
    auto memslc = mem_span.slice(128, 128);
    lidl::message_builder builder(memslc);

    auto& cap = lidl::create<tos::path_capability>(
        builder, lidl::create_string(builder, "/home/meow/foo"), tos::permissions2::read);

    uint32_t ptr = std::distance(mem, reinterpret_cast<uint8_t*>(&cap));
    const void* args[] = {&ptr};

    [[maybe_unused]] auto res = m3_Call(run, 1, args);

    bool result;
    const void* ret_ptrs[] = {&result};
    m3_GetResults(run, 1, ret_ptrs);

    tos::println(com, "Run result:", result);
}

void tos_main() {
    tos::launch(tos::alloc_stack, wasm_task);
}