#include <calc_generated.hpp>
#include <common/xbee.hpp>
#include <tos/ae/transport/hosted/udp.hpp>
#include <tos/ae/transport/xbee/xbee_host.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;

struct calc : tos::ae::services::calculator::sync_server {
    int32_t add(const int32_t& x, const int32_t& y) override {
        return x + y;
    }
    float mul(const float& x, const float& y) override {
        return x * y;
    }
    float fma(const float& x, const float& y, const float& z) override {
        return x * y + z;
    }
};

void main_task() {
    auto com = bs::default_com::open();
    tos::debug::serial_sink uart_sink(&com);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    tos::debug::log("hi");

    auto timer = tos::open(tos::devs::timer<0>);
    tos::alarm alarm{&timer};

    tos::ae::services::calculator::stub_client<tos::ae::hosted_udp_transport> remote_calc(
        tos::udp_endpoint_t{tos::parse_ipv4_address("138.68.28.13"),
                            tos::port_num_t{1999}});

    calc local_calc;
    tos::ae::sync_service_host host(&local_calc);

    auto xbee_uart = bs::xbee_uart::open();

    tos::ae::xbee_exporter<tos::hosted::usart&, decltype((alarm))> exporter(xbee_uart,
                                                                            alarm);

    exporter.export_service(host, tos::ae::xbee_export_args{.channel = 0});

    tos::this_thread::block_forever();
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 4> stk;
void tos_main() {
    tos::launch(stk, main_task);
}