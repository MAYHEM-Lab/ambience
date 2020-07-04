#include "remote_service.hpp"
#include "service_handler.hpp"

#include <arch/drivers.hpp>
#include <service_generated.hpp>
#include <tos/build.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/serial_packets.hpp>

class sys_server : public tos::services::system_status {
public:
    std::string_view get_commit_hash(lidl::message_builder& response_builder) override {
        return tos::build::commit_hash();
    }
    std::string_view get_build_id(lidl::message_builder& response_builder) override {
        return "";
    }
    std::string_view get_arch(lidl::message_builder& response_builder) override {
        return tos::build::arch();
    }
};

using generated_remote_system = tos::services::remote_system_status<packet_transport>;

void query_sys(tos::services::system_status& server) {
    LOG("Service name:", server.name());
    std::array<uint8_t, 128> resp_buf;
    lidl::message_builder mb(resp_buf);
    LOG("Commit hash:", server.get_commit_hash(mb));
    mb = lidl::message_builder(resp_buf);
    LOG("Arch:", server.get_arch(mb));
    mb = lidl::message_builder(resp_buf);
    LOG("Build id:", server.get_build_id(mb));
}

void service_main() {
    sys_server server;
    query_sys(server);

    auto link = std::make_unique<tos::hosted::usart>(
        get_io(), "/dev/ttyACM0", tos::uart::default_115200);
    auto transport = tos::io::serial_packets{std::move(link)};

    tos::launch(tos::alloc_stack, [&] {
        tos::debug::serial_sink sink(tos::hosted::stderr_adapter{}, "remote");
        tos::debug::log_server log_server(sink);

        auto rep_handler = make_request_handler<tos::services::logger>();
        auto channel = transport.get_channel(4);
        while (true) {
            auto packet = channel->receive();
            std::array<uint8_t, 256> resp_buf;
            lidl::message_builder build(resp_buf);
            rep_handler(log_server, lidl::buffer{packet->data()}, build);
            auto response = build.get_buffer().get_buffer().slice(0, build.size());
            channel->send(response);
        }
    });

    generated_remote_system remote_sys{transport.get_channel(3)};
    query_sys(remote_sys);

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::hosted::stderr_adapter{}),
            tos::hosted::clock<std::chrono::system_clock>{}}));
    tos::launch(tos::alloc_stack, service_main);
}