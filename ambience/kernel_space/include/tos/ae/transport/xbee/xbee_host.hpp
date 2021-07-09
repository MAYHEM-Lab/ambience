#pragma once

#include <common/xbee.hpp>
#include <tos/ae/detail/handle_req.hpp>
#include <tos/ae/exporter.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/cancellation_token.hpp>
#include <tos/ft.hpp>
#include <tos/intrusive_list.hpp>

namespace tos::ae {
struct xbee_export_args : export_args {
    int channel;
};

template<class SerialType, class AlarmT>
class xbee_exporter : public exporter {
public:
    xbee_exporter(SerialType serial, AlarmT alarm)
        : m_serial{serial}
        , m_alarm{alarm} {
        tos::launch(tos::alloc_stack, [this] { read_thread(); });
    }

private:
    template<class HostType>
    struct host : list_node<host<HostType>> {
        host(int channel, const HostType& h)
            : m_channel(channel)
            , m_host(h) {
        }
        int m_channel;
        HostType m_host;
    };

    void run_message_on_channel(int channel,
                                tos::span<uint8_t> msg,
                                lidl::message_builder& response_builder) {
        for (auto& h : m_sync_hosts) {
            if (h.m_channel == channel) {
                sync_run_message(h.m_host, msg, response_builder);
            }
        }
        for (auto& h : m_async_hosts) {
            if (h.m_channel == channel) {
                sync_run_message(h.m_host, msg, response_builder);
            }
        }
    }

    void read_thread() {
        while (!cancellation_token::system().is_cancelled()) {
            auto recv_retry = [&] {
                while (true) {
                    if (auto recv_res = tos::xbee::receive(m_serial, m_alarm)) {
                        return force_get(std::move(recv_res));
                    }
                }
            };
            auto p = recv_retry();

            std::array<uint8_t, 2048> buf;
            lidl::message_builder rb{buf};

            run_message_on_channel(0, tos::const_span_cast(p.data()), rb);

            tos::xbee_s1 x(m_serial);
            tos::xbee::tx16_req req{p.from(), rb.get_buffer(), tos::xbee::frame_id_t{1}};
            x.transmit(req);

            using namespace std::chrono_literals;
            int retries = 5;
            tos::xbee::tx_status stat;
            while (retries-- > 0) {
                auto tx_r = tos::xbee::read_tx_status(m_serial, m_alarm);
                if (tx_r) {
                    stat = tos::force_get(tx_r);
                    break;
                }
            }
            tos::debug::log(int(stat.status));
        }
    }

    intrusive_list<host<sync_service_host>> m_sync_hosts;
    intrusive_list<host<async_service_host>> m_async_hosts;
    SerialType m_serial;
    AlarmT m_alarm;

public:
    void export_service(const sync_service_host& h, const export_args& args) override {
        m_sync_hosts.push_back(*new host<sync_service_host>(
            static_cast<const xbee_export_args&>(args).channel, h));
    }

    void export_service(const async_service_host& h, const export_args& args) override {
        m_async_hosts.push_back(*new host<async_service_host>(
            static_cast<const xbee_export_args&>(args).channel, h));
    }
};
} // namespace tos::ae