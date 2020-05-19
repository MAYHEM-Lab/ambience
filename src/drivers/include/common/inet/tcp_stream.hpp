//
// Created by fatih on 6/30/18.
//

#pragma once

#include <arch/tcp.hpp>
#include <common/driver_base.hpp>
#include <common/inet/lwip.hpp>
#include <tos/event.hpp>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

namespace tos {
enum class read_error
{
    disconnected,
    timeout
};

template<class BaseEndpointT>
class tcp_stream : public self_pointing<tcp_stream<BaseEndpointT>> {
public:
    explicit tcp_stream(BaseEndpointT&& ep);

    int write(span<const uint8_t>);

    expected<span<uint8_t>, read_error> read(span<uint8_t>);

    void operator()(lwip::events::sent_t, BaseEndpointT&, uint16_t);
    void operator()(lwip::events::discon_t, BaseEndpointT&, lwip::discon_reason);
    void operator()(lwip::events::recv_t, BaseEndpointT&, lwip::buffer&&);

    ALWAYS_INLINE
    bool disconnected() const {
        return m_discon;
    }

private:
    void attach();

    lwip::buffer m_buffer;

    BaseEndpointT m_ep;
    tos::mutex m_busy;
    tos::semaphore m_write_sync{0};
    bool m_discon{false};
    uint16_t m_sent_bytes = 0;
};

template<class EndPointT>
tcp_stream<EndPointT> make_stream(EndPointT&& ep) {
    return tcp_stream<EndPointT>{std::move(ep)};
}

template<class BaseEndpointT>
ALWAYS_INLINE tcp_stream<BaseEndpointT>::tcp_stream(BaseEndpointT&& ep)
    : m_ep(std::move(ep)) {
    attach();
}

template<class BaseEndpointT>
inline void tcp_stream<BaseEndpointT>::attach() {
    m_ep.attach(*this);
}

template<class BaseEndpointT>
inline void tcp_stream<BaseEndpointT>::operator()(tos::lwip::events::sent_t,
                                                  BaseEndpointT&,
                                                  uint16_t len) {
#ifdef ESP_TCP_VERBOSE
    tos_debug_print("sent: %d\n", int(len));
#endif
    LOG_TRACE("Sent:", len);
    m_sent_bytes += len;
    m_write_sync.up();
}

template<class BaseEndpointT>
inline void tcp_stream<BaseEndpointT>::operator()(tos::lwip::events::discon_t,
                                                  BaseEndpointT&,
                                                  lwip::discon_reason r) {
    m_discon = true;
    m_write_sync.up();
    m_buffer.eof();
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("closed: %d\n", int(r));
#endif
}

template<class BaseEndpointT>
int tcp_stream<BaseEndpointT>::write(tos::span<const uint8_t> buf) {
    tos::lock_guard lk{m_busy};
    if (m_discon) {
        return 0;
    }
    m_sent_bytes = 0;
    auto to_send = m_ep.send(buf);
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("sending: %d\n", int(to_send));
#endif
    while (m_sent_bytes != to_send && !m_discon) {
        m_write_sync.down();
    }
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("sentt: %d\n", int(m_sent_bytes));
#endif
    return m_sent_bytes;
}

template<class BaseEndpointT>
inline void tcp_stream<BaseEndpointT>::operator()(lwip::events::recv_t,
                                                  BaseEndpointT&,
                                                  lwip::buffer&& buf) {
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("recved %d\n", buf.size());
#endif
    if (m_buffer.has_more()) {
        m_buffer.append(std::move(buf));
    } else {
        m_buffer = std::move(buf);
    }
}

template<class BaseEndpointT>
expected<span<uint8_t>, read_error>
tcp_stream<BaseEndpointT>::read(tos::span<uint8_t> to) {
    if (m_discon && m_buffer.size() == 0) {
        return unexpected(read_error::disconnected);
    }

    tos::lock_guard<tos::mutex> lk{m_busy};

    auto res = m_buffer.read(to);

    if (m_discon && res.size() == 0) {
        return unexpected(read_error::disconnected);
    }

    return res;
}
} // namespace tos