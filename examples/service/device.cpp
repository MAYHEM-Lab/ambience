#include "remote_service.hpp"
#include "service_handler.hpp"

#include <arch/drivers.hpp>
#include <log_generated.hpp>
#include <service_generated.hpp>
#include <tos/build.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/io/serial_packets.hpp>

class remote_log
    : public logger
    , public lidl::remote_service {
public:
    using remote_service::remote_service;

    bool start(const log_level& level) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_start_params{level});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res = lidl::get_root<return_union>(lidl::buffer(response->data())).start();
        return res.ret0();
    }

    bool finish() override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_finish_params{});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res = lidl::get_root<return_union>(lidl::buffer(response->data())).finish();
        return res.ret0();
    }

    bool log_int(const int64_t& val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_log_int_params{val});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_int();
        return res.ret0();
    }

    bool log_float(const double& val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_log_float_params{val});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_float();
        return res.ret0();
    }

    bool log_bool(const bool& val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_log_bool_params{val});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_bool();
        return res.ret0();
    }

    bool log_string(std::string_view val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        auto& str = create_string(mb, val);
        lidl::create<call_union>(mb, lidl::create<logger_log_string_params>(mb, str));
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_string();
        return res.ret0();
    }

    bool log_pointer(const uint64_t& val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_log_pointer_params{val});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_pointer();
        return res.ret0();
    }

    bool log_log_level(const log_level& val) override {
        using namespace lidl;
        std::array<uint8_t, 64> data;
        message_builder mb(data);
        lidl::create<call_union>(mb, logger_log_log_level_params{val});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).log_log_level();
        return res.ret0();
    }
};

class sys_server : public system_status {
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

template<class ServiceT>
class registered_service;

class registered_service_base : public tos::list_node<registered_service_base> {
public:
    std::string name;

    template<class ServiceT>
    registered_service<ServiceT>& as() {
        return *static_cast<registered_service<ServiceT>*>(this);
    }
};

template<class ServiceT>
class registered_service : public registered_service_base {
public:
    ServiceT service;
};

class service_registry {
public:
    template<class ServT>
    ServT* get_service(std::string_view name) {
        auto ptr = get_service_base(name);
        if (!ptr) {
            return nullptr;
        }
        return &ptr->as<ServT>().service;
    }

    void register_service(registered_service_base& service) {
        m_services.push_back(service);
    }

private:
    registered_service_base* get_service_base(std::string_view name) const {
        auto it = std::find_if(m_services.begin(), m_services.end(), [&name](auto& srv) {
            return srv.name == name;
        });
        if (it == m_services.end()) {
            return nullptr;
        }
        return &*it;
    }

    tos::intrusive_list<registered_service_base> m_services;
};

void service_task() {
    using namespace tos::tos_literals;

    auto link = tos::open(tos::devs::usart<1>, tos::uart::default_115200, 23_pin, 22_pin);
    auto transport = tos::io::serial_packets{&link};

    registered_service<sys_server> server;
    service_registry services;
    services.register_service(server);

    tos::launch(tos::alloc_stack, [&] {
        remote_log log(transport.get_channel(4));
        tos::debug::lidl_sink sink(log);
        tos::debug::detail::any_logger logger(&sink);
        logger.set_log_level(tos::debug::log_level::log);
        tos::debug::set_default_log(&logger);

        LOG("Hello world!");
    });

    auto rep_handler = make_request_handler<system_status>();
    auto channel = transport.get_channel(3);
    while (true) {
        auto packet = channel->receive();
        std::array<uint8_t, 256> resp_buf;
        lidl::message_builder build(resp_buf);
        rep_handler(server.service, lidl::buffer{packet->data()}, build);
        auto response = build.get_buffer().get_buffer().slice(0, build.size());
        channel->send(response);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, service_task);
}