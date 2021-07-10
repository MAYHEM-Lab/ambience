#include <calc_generated.hpp>
#include <echo_generated.hpp>
#include <log_generated.hpp>
#include <common/xbee.hpp>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/transport/xbee/xbee_transport.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;

void main_task() {
    auto com = bs::default_com::open();
    tos::debug::serial_sink uart_sink(&com);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    auto xbee_uart = bs::xbee_uart::open();

    auto timer = tos::open(tos::devs::timer<2>);
    tos::alarm alarm{&timer};

    auto g = tos::open(tos::devs::gpio);
    auto reset_pin = tos::stm32::instantiate_pin(3);
    g.set_pin_mode(reset_pin, tos::pin_mode::out);
    g.write(reset_pin, tos::digital::high);

    using namespace std::chrono_literals;
    tos::this_thread::sleep_for(alarm, 50ms);

    g.write(reset_pin, tos::digital::low);
    tos::this_thread::sleep_for(alarm, 50ms);

    g.write(reset_pin, tos::digital::high);
    tos::this_thread::sleep_for(alarm, 50ms);

    auto r = tos::xbee::read_modem_status(xbee_uart, alarm);
    tos::debug::log(bool(r));

    tos::ae::xbee_importer importer(&xbee_uart, &alarm);

    auto c = importer.import_service<tos::ae::services::echo>(
        tos::ae::xbee_import_args{.addr = tos::xbee::addr_16{0x1234}, .channel = 0});

    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            tos::debug::log(i, "+", j, "=", c->echo_num(i + j));
        }
    }
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 4> stk;
void tos_main() {
    tos::launch(stk, main_task);
}