#include "tos/debug/dynamic_log.hpp"
#include "tos/debug/sinks/remote_sink.hpp"
#include "tos/debug/sinks/serial_sink.hpp"
#include "tos/ft.hpp"
#include "tos/intrusive_ptr.hpp"
#include "tos/io/packet.hpp"
#include "tos/io/serial_multiplexer.hpp"
#include "tos/span.hpp"
#include "tos/stack_storage.hpp"
#include "tos/thread.hpp"
#include "tos/utest/test.hpp"

#include <arch/drivers.hpp>
#include <string_view>
#include <tos/build.hpp>
#include <tos/io/serial_packets.hpp>

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

constexpr auto board = boards::stm32l4_disco{};

namespace {
tos::stack_storage test_stack;
}
void tos_main() {
    tos::launch(test_stack, [] {
        // Run tests somehow
        auto usart = board.open_dev_serial();
        tos::io::serial_packets packets(&usart);
        tos::debug::remote_sink sink(packets);
        tos::debug::detail::any_logger logger(&sink);
        tos::debug::set_default_log(&logger);

        LOG("Boot complete");
        LOG("UTest Initialized");

        for (auto& test : tos::utest::all_tests) {
            LOG(test.name);
            LOG(test.run());
        }

        LOG("UTest Finished");
        tos::this_thread::block_forever();
    });
}