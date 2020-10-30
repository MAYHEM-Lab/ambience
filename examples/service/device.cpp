#include "remote_service.hpp"
#include <arch/drivers.hpp>
#include <log_generated.hpp>
#include <map>
#include <service_generated.hpp>
#include <tos/build.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/io/serial_backend.hpp>

template<class SerialT>
class serial_channel : public tos::io::any_channel {
public:
    serial_channel(tos::io::serial_backend<SerialT>& backend, int stream)
        : m_backend{&backend}
        , m_stream_id{stream} {
    }

    void send(tos::span<const uint8_t> span) override {
        m_backend->send(m_stream_id, span);
    }

    tos::intrusive_ptr<tos::io::packet> receive() override {
        m_wait.down();
        return std::move(m_packet);
    }

    void receive(tos::intrusive_ptr<tos::io::packet> packet) {
        m_packet = std::move(packet);
        m_wait.up();
    }

private:
    tos::io::serial_backend<SerialT>* m_backend;
    int m_stream_id;

    tos::intrusive_ptr<tos::io::packet> m_packet;
    tos::semaphore m_wait{0};
};

using generated_remote_logger = tos::services::remote_logger<packet_transport>;

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

class registered_service_base {
public:
    lidl::service_base* service;
    lidl::procedure_runner_t call;

    void invoke(tos::span<const uint8_t> data, lidl::message_builder& builder) {
        call(*service, data, builder);
    }
};

class service_registry {
public:
    template<class ServT>
    void register_service(lidl::service_base& service) {
        m_services.push_back({&service, lidl::make_procedure_runner<ServT>()});
    }

    std::vector<registered_service_base> m_services;
};

struct registry_discovery : tos::services::discovery {
    service_registry* registry;
    int16_t service_count() override {
        return registry->m_services.size();
    }

    const lidl::vector<tos::services::service_info>&
    services(lidl::message_builder& response_builder) override {
        auto& res =
            lidl::create_vector_sized<tos::services::service_info>(response_builder, 2);
        res.span()[0] = tos::services::service_info{1, 42, 65};
        res.span()[1] = tos::services::service_info{3, 145, 412345};
        return res;
    }
};

void service_task() {
    using namespace tos::tos_literals;

    auto link = tos::open(tos::devs::usart<1>, tos::uart::default_115200, 23_pin, 22_pin);

    service_registry services;

    registry_discovery disc;
    disc.registry = &services;
    services.register_service<tos::services::discovery>(disc);

    sys_server server;
    services.register_service<tos::services::system_status>(server);

    tos::io::serial_backend transport(&link);
    auto log_channel =
        tos::make_intrusive<serial_channel<decltype(transport)::serial_type>>(transport,
                                                                              4);

    generated_remote_logger log(log_channel);
    tos::debug::lidl_sink sink(log);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::log);
    tos::debug::set_default_log(&logger);

    tos::launch(tos::alloc_stack, [&] {
        tos::io::poll_packets(transport,
                              tos::cancellation_token::system(),
                              [&](int stream, tos::intrusive_ptr<tos::io::packet>&& p) {
                                  if (stream == 4) {
                                      log_channel->receive(std::move(p));
                                      return;
                                  }

                                  std::array<uint8_t, 256> resp_buf;
                                  lidl::message_builder build(resp_buf);

                                  switch (stream) {
                                  case 1:
                                      services.m_services[0].invoke(as_span(*p), build);
                                      break;
                                  case 3:
                                      services.m_services[1].invoke(as_span(*p), build);
                                      break;
                                  default:
                                      return;
                                  }

                                  auto response =
                                      build.get_buffer().slice(0, build.size());
                                  transport.send(stream, response);
                              });
    });

    LOG("Hello world!");
}

void tos_main() {
    tos::launch(tos::alloc_stack, service_task);
}