#pragma once

#include "common/xbee/response.hpp"
#include "tos/span.hpp"
#include <common/xbee.hpp>
#include <cstring>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/importer.hpp>
#include <tos/mutex.hpp>

namespace tos::ae {
struct xbee_import_args : import_args {
    tos::xbee::addr_16 addr;
    int channel;
};

struct xbee_tx_buf {
    xbee_tx_buf(size_t buf_sz, uint32_t seq_no)
        : full_buffer(buf_sz) {
        memcpy(full_buffer.data(), &seq_no, sizeof seq_no);
    }
    std::vector<uint8_t> full_buffer;
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
            tos::xbee::tx16_req req{m_address,
                                    span<uint8_t>(buf.begin() - sizeof(uint32_t),
                                                  buf.size() + sizeof(uint32_t)),
                                    tos::xbee::frame_id_t{1}};

            tos::xbee_s1 x{meta::deref(m_back_ptr->m_ser)};

            for (int i = 0; i < 5; ++i) {
                x.transmit(req);

                int retries = 5;
                tos::xbee::tx_status stat;
                stat.status = tos::xbee::tx_status::statuses::no_ack;
                while (retries-- > 0) {
                    auto tx_r = tos::xbee::read_tx_status(
                        meta::deref(m_back_ptr->m_ser), meta::deref(m_back_ptr->m_alarm));
                    if (tx_r) {
                        stat = tos::force_get(tx_r);
                        break;
                    }
                }
                if (stat.status != tos::xbee::tx_status::statuses::success) {
                    LOG_WARN("Transmit failed");
                    continue;
                }

                using namespace std::chrono_literals;

                for (int j = 0; j < 5; ++j) {
                    auto packet = tos::xbee::receive(meta::deref(m_back_ptr->m_ser),
                                                     meta::deref(m_back_ptr->m_alarm),
                                                     500ms);

                    if (packet) {
                        m_last_packet = std::move(force_get(packet));
                        goto got_packet;
                    }
                }
            }

        got_packet:

            auto packet_data = m_last_packet.data();
            uint32_t seq;
            memcpy(&seq, packet_data.data(), sizeof seq);
            // LOG("Received seq", seq);

            return tos::const_span_cast(packet_data.slice(sizeof seq));
        }

        uint32_t next_seq_no = 1;
        xbee_tx_buf get_buffer() {
            // LOG("Sending seq", next_seq_no);
            return xbee_tx_buf(128, next_seq_no++);
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

template<>
inline tos::span<uint8_t> lidl::as_span(tos::ae::xbee_tx_buf& buf) {
    return tos::span<uint8_t>(buf.full_buffer).slice(sizeof(uint32_t));
}