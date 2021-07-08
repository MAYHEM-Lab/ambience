#include <calc_generated.hpp>
#include <common/xbee.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;
struct sync_service_host {
    template<class ServiceT>
    explicit sync_service_host(ServiceT* serv)
        : impl{serv}
        , message_runner{lidl::make_erased_procedure_runner<
              typename ServiceT::service_type::sync_server>()} {
    }

    bool run_message(tos::span<uint8_t> buffer,
                     lidl::message_builder& response_builder) const {
        return message_runner(*impl, buffer, response_builder);
    }

    lidl::sync_service_base* impl;
    lidl::erased_procedure_runner_t message_runner;
};

struct udp_transport {
    udp_transport(tos::udp_endpoint_t ep)
        : ep{ep} {
    }

    std::vector<uint8_t> get_buffer() {
        return std::vector<uint8_t>(1024);
    }

    std::vector<uint8_t> send_receive(tos::span<uint8_t> data) {
        sock.send_to(data, ep);

        std::vector<uint8_t> buf(1024);
        tos::udp_endpoint_t from_ep;
        auto res = sock.receive_from(buf, from_ep);
        auto d = force_get(res);
        buf.resize(d.size());
        return buf;
    }

    template<class FnT>
    auto& transform_call(lidl::message_builder&, const FnT& fn) {
        return fn();
    }

    template<class RetT>
    const RetT& transform_return(tos::span<const uint8_t> buf) {
        return lidl::get_root<RetT>(buf);
    }

    tos::udp_endpoint_t ep;
    tos::hosted::udp_socket sock{get_io()};
};

struct calc : tos::ae::services::calculator::sync_server {
    int32_t add(const int32_t& x, const int32_t& y) override {
        return x + y;
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

    tos::ae::services::calculator::stub_client<udp_transport> remote_calc(
        tos::udp_endpoint_t{tos::parse_ipv4_address("138.68.28.13"),
                            tos::port_num_t{1999}});

    calc local_calc;
    sync_service_host host(&local_calc);

    tos::hosted::usart xbee_uart{get_io(), "/dev/ttyUSB1", tos::uart::default_38400};

    tos::xbee_s1 x{xbee_uart};

    while (true) {
        auto recv_retry = [&] {
            while (true) {
                if (auto recv_res = tos::xbee::receive(xbee_uart, alarm)) {
                    return force_get(std::move(recv_res));
                }
            }
        };
        auto p = recv_retry();

        std::array<uint8_t, 2048> buf;
        lidl::message_builder rb{buf};
        host.run_message(tos::const_span_cast(p.data()), rb);

        tos::xbee::tx16_req req{p.from(), rb.get_buffer(), tos::xbee::frame_id_t{1}};
        x.transmit(req);

        using namespace std::chrono_literals;
        int retries = 5;
        tos::xbee::tx_status stat;
        while (retries-- > 0) {
            auto tx_r = tos::xbee::read_tx_status(xbee_uart, alarm);
            if (tx_r) {
                stat = tos::force_get(tx_r);
                break;
            }
        }
        tos::debug::log(int(stat.status));
    }
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 4> stk;
void tos_main() {
    tos::launch(stk, main_task);
}