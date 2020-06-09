#include <arch/drivers.hpp>
#include <service_generated.hpp>
#include <tos/build.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/serial_packets.hpp>

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

class log_server : public logger {
public:
    explicit log_server(tos::debug::detail::any_sink& sink)
        : m_sink{&sink} {
    }

    bool start(const log_level& level) override {
        tos::debug::log_level lvl;
        switch (level) {
        case log_level::trace:
            lvl = tos::debug::log_level::trace;
            break;
        case log_level::debug:
            lvl = tos::debug::log_level::debug;
            break;
        case log_level::info:
            lvl = tos::debug::log_level::info;
            break;
        }
        return m_sink->begin(lvl);
    }
    bool finish() override {
        m_sink->end();
        return true;
    }
    bool log_int(const int64_t& val) override {
        m_sink->add(val);
        return true;
    }
    bool log_float(const double& val) override {
        m_sink->add(val);
        return true;
    }
    bool log_bool(const bool& val) override {
        m_sink->add(val);
        return true;
    }
    bool log_string(std::string_view val) override {
        m_sink->add(val);
        return true;
    }
    bool log_pointer(const uint64_t& val) override {
        m_sink->add(reinterpret_cast<void*>(val));
        return true;
    }

private:
    tos::debug::detail::any_sink* m_sink;
};

class remote_service {
public:
    explicit remote_service(tos::intrusive_ptr<tos::io::any_channel> channel)
        : m_channel{std::move(channel)} {
    }

protected:
    tos::intrusive_ptr<tos::io::packet> send_receive(tos::span<const uint8_t> send_buf) {
        m_channel->send(send_buf);
        return m_channel->receive();
    }

    tos::intrusive_ptr<tos::io::any_channel> m_channel;
};

class remote_system
    : public system_status
    , public remote_service {
public:
    using remote_service::remote_service;

    std::string_view get_commit_hash(lidl::message_builder& response_builder) override {
        std::array<uint8_t, 128> buffer;
        lidl::message_builder mb{buffer};
        lidl::create<call_union>(mb, system_status_get_commit_hash_params{});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res = lidl::get_root<return_union>(lidl::buffer(response->data()))
                        .get_commit_hash();
        auto& str_res = lidl::create_string(response_builder, res.ret0());
        return str_res;
    }

    std::string_view get_build_id(lidl::message_builder& response_builder) override {
        std::array<uint8_t, 128> buffer;
        lidl::message_builder mb{buffer};
        lidl::create<call_union>(mb, system_status_get_build_id_params{});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).get_build_id();
        auto& str_res = lidl::create_string(response_builder, res.ret0());
        return str_res;
    }

    std::string_view get_arch(lidl::message_builder& response_builder) override {
        std::array<uint8_t, 128> buffer;
        lidl::message_builder mb{buffer};
        lidl::create<call_union>(mb, system_status_get_arch_params{});
        auto packet = mb.get_buffer().get_buffer().slice(0, mb.size());
        auto response = send_receive(packet);
        auto& res =
            lidl::get_root<return_union>(lidl::buffer(response->data())).get_arch();
        auto& str_res = lidl::create_string(response_builder, res.ret0());
        return str_res;
    }
};

void query_sys(system_status& server) {
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

    remote_system remote_sys{transport.get_channel(3)};
    query_sys(remote_sys);
}

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::hosted::stderr_adapter{}),
            tos::hosted::clock<std::chrono::system_clock>{}}));
    tos::launch(tos::alloc_stack, service_main);
}