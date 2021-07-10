#pragma once

#include <common/xbee.hpp>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/importer.hpp>
#include <tos/mutex.hpp>

namespace tos::ae {
struct xbee_import_args : import_args {
    tos::xbee::addr_16 addr;
    int channel;
};

template<class SerialType, class AlarmType>
class xbee_importer : public importer::sync_server {
    struct transport : lidl::verbatim_transform {
        transport(xbee_importer* back_ptr, tos::xbee::addr_16 addr, int channel)
            : m_back_ptr{back_ptr}
            , m_address(addr)
            , m_channel(channel) {
        }

        tos::span<uint8_t> send_receive(tos::span<uint8_t> buf) {
            tos::lock_guard lg{m_back_ptr->m_mutex};
            m_back_ptr->m_num_calls++;
            tos::xbee::tx16_req req{m_address, buf, tos::xbee::frame_id_t{1}};

            tos::xbee_s1 x{meta::deref(m_back_ptr->m_ser)};
            x.transmit(req);

            int retries = 5;
            tos::xbee::tx_status stat;
            while (retries-- > 0) {
                auto tx_r =
                    tos::xbee::read_tx_status(meta::deref(m_back_ptr->m_ser), meta::deref(m_back_ptr->m_alarm));
                if (tx_r) {
                    stat = tos::force_get(tx_r);
                    break;
                }
            }

            m_last_packet = force_get(tos::xbee::receive(
                meta::deref(m_back_ptr->m_ser), meta::deref(m_back_ptr->m_alarm)));

            return tos::const_span_cast(m_last_packet.data());
        }

        std::vector<uint8_t> get_buffer() {
            return std::vector<uint8_t>(128);
        }

        xbee_importer* m_back_ptr;
        tos::xbee::addr_16 m_address;
        int m_channel;
        tos::xbee::recv16 m_last_packet;
    };

public:
    xbee_importer(SerialType serial, AlarmType alarm)
        : m_ser{serial}
        , m_alarm{alarm} {
    }

private:
    int64_t number_of_calls() override {
        return m_num_calls;
    }

public:
    template<class ServiceType>
    typename ServiceType::sync_server* import_service(const xbee_import_args& args) {
        return new typename ServiceType::template stub_client<transport>(
            this, args.addr, args.channel);
    }

private:
    tos::mutex m_mutex;
    SerialType m_ser;
    AlarmType m_alarm;
    int64_t m_num_calls = 0;
};
} // namespace tos::ae
