#include <calc_generated.hpp>
#include <common/xbee.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;

tos::Task<tos::xbee::recv16> xbee_receive(tos::stm32::usart& uart) {
    tos::xbee::sm_response_parser<tos::xbee::recv16,
                                  tos::xbee::recv_alloc<tos::xbee::recv16>>
        parser({});
    tos::semaphore sem{0};

    auto char_handler = [&](uint8_t data) {
        parser.consume(data);
        if (parser.finished()) {
            sem.up_isr();
        }
    };

    uart.set_read_cb(tos::function_ref<void(uint8_t)>(char_handler));

    co_await sem;

    uart.reset_read_cb();
    co_return std::move(parser).get_payload();
}

template<class Serial, class Alarm>
struct xbee_transport {
    xbee_transport(tos::xbee::addr_16 addr, Serial ser, Alarm alarm)
        : m_address(addr)
        , m_ser{ser}
        , m_alarm{alarm} {
    }

    tos::span<uint8_t> send_receive(tos::span<uint8_t> buf) {
        tos::xbee::tx16_req req{m_address, buf, tos::xbee::frame_id_t{1}};

        tos::xbee_s1 x{m_ser};
        x.transmit(req);

        int retries = 5;
        tos::xbee::tx_status stat;
        while (retries-- > 0) {
            auto tx_r = tos::xbee::read_tx_status(m_ser, m_alarm);
            if (tx_r) {
                stat = tos::force_get(tx_r);
                break;
            }
        }

        tos::semaphore wait{0};
        tos::coro::make_detached(xbee_receive(m_ser), [&](auto packet) {
            p = std::move(packet);
            wait.up();
        });

        wait.down();
        return tos::const_span_cast(p.data());
    }

    std::vector<uint8_t> get_buffer() {
        return std::vector<uint8_t>(128);
    }

    template<class FnT>
    auto& transform_call(lidl::message_builder&, const FnT& fn) {
        return fn();
    }

    template<class RetT>
    const RetT& transform_return(tos::span<const uint8_t> buf) {
        return lidl::get_root<RetT>(buf);
    }

    tos::xbee::recv16 p;
    tos::xbee::addr_16 m_address;
    Serial m_ser;
    Alarm m_alarm;
};

void main_task() {
    auto com = bs::default_com::open();
    tos::debug::serial_sink uart_sink(&com);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    auto xbee_uart = bs::usart4::open();

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

    using transport = xbee_transport<tos::stm32::usart&, decltype(alarm)&>;
    using client = tos::ae::services::calculator::stub_client<transport>;

    client c(tos::xbee::addr_16{0x1234}, xbee_uart, alarm);

    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            tos::debug::log(i, "+", j, "=", c.add(i, j));
        }
    }
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 4> stk;
void tos_main() {
    tos::launch(stk, main_task);
}