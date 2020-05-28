//
// Created by fatih on 10/17/19.
//

#include "tos/debug/debug.hpp"
#include "tos/expected.hpp"
#include "tos/thread.hpp"

#include <arch/drivers.hpp>
#include <arch/mpu.hpp>
#include <optional>
#include <tos/barrier.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/memory.hpp>

namespace boards {
#if defined(TOS_PLATFORM_stm32_hal)
template<class BoardT>
struct stm32_dev_common {
    auto open_led_gpio();
    static auto open_dev_serial() {
        using namespace tos;
        using namespace tos_literals;

        auto usart_rx_pin = operator""_pin(BoardT::rx_pin);
        auto usart_tx_pin = operator""_pin(BoardT::tx_pin);

        return open(
            BoardT::vcp_dev, tos::uart::default_115200, usart_rx_pin, usart_tx_pin);
    }
};

struct stm32f7_disco : stm32_dev_common<stm32f7_disco> {
    static constexpr int led_pin = ('i' - 'a') * 16 + 1;
    static constexpr int tx_pin = 9;
    static constexpr int rx_pin = 23;
    static constexpr auto& vcp_dev = tos::devs::usart<1>;
};
struct stm32l4_disco : stm32_dev_common<stm32l4_disco> {
    static constexpr int led_pin = 5;
    static constexpr int tx_pin = 22;
    static constexpr int rx_pin = 23;
    static constexpr auto& vcp_dev = tos::devs::usart<1>;
};
#endif

#if defined(TOS_PLATFORM_cc32xx)
struct cc3220sf {
    static auto open_dev_serial() {
        return tos::cc32xx::uart(0);
    }
};
#endif
} // namespace boards

constexpr auto board = boards::stm32f7_disco{};

alignas(64) char foo[256];

alignas(64) tos::stack_storage stack;

volatile int* ptr;

[[gnu::optimize("O0")]] int recursive_sum(int n) {
    if (n == 0) {
        return 0;
    }

    return n + recursive_sum(n - 1);
}

void mpu_task() {
    using namespace tos;
    using namespace tos_literals;

    auto usart_rx_pin = operator""_pin(board.rx_pin);
    auto usart_tx_pin = operator""_pin(board.tx_pin);

    auto usart =
        open(devs::usart<1>, tos::uart::default_115200, usart_rx_pin, usart_tx_pin);
    debug::serial_sink sink(&usart);
    debug::detail::any_logger logger(&sink);

    tos::debug::set_default_log(&logger);

    tos::arm::mpu mpu;
    auto regs = mpu.num_supported_regions();
    LOG("MPU Number of regions:", int(regs));

    auto min_size = mpu.min_region_size();
    LOG("MPU Minimum region size:", int(min_size));

    Assert(!mpu.get_region(0));

    LOG("Marking 64 bytes as inaccessible at 0");
    mpu.set_region(1, {0, 32}, tos::permissions::none);
    Assert(mpu.get_region(1));

    //    LOG("Accessing nullptr!");
    //    *ptr = 42;

    memory_region end_of_stack{reinterpret_cast<uintptr_t>(&stack), 32};
    LOG("Marking [",
        (void*)end_of_stack.base,
        ",",
        (void*)end_of_stack.end(),
        ") as inaccessible");
    mpu.set_region(2, end_of_stack, tos::permissions::none);
    Assert(mpu.get_region(2));

    //    LOG("Stack overflow!");
    //    recursive_sum(10000);

    LOG("Marking", 128, "bytes as inaccessible at", &foo);
    auto set_res =
        mpu.set_region(0,
                       {.base = reinterpret_cast<uintptr_t>(&foo), .size = 128},
                       tos::permissions::none);
    Assert(set_res);
    auto region = mpu.get_region(0);
    Assert(region);
    LOG("Region:", reinterpret_cast<void*>(region->base), ",", region->size);

    LOG("Attempting access to", static_cast<void*>(&foo[128]));
    tos::debug::do_not_optimize(foo[128] = 5);
    LOG("Passed the first one");

    LOG("Attempting access to", static_cast<void*>(&foo[0]));
    tos::debug::do_not_optimize(foo[0] = 5);
    LOG("Passed the second one");

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(stack, mpu_task);
}
