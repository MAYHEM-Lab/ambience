//
// Created by fatih on 6/30/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <tos/condition_variable.hpp>
#include <tos/event.hpp>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/lwip/lwip.hpp>
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
    tos::Task<int> async_write(span<const uint8_t>);
    tos::Task<int> async_send(span<const uint8_t>);


    expected<span<uint8_t>, read_error> read(span<uint8_t>);

    tos::Task<expected<lwip::buffer, read_error>> async_read_buffer();

    void operator()(lwip::events::sent_t, BaseEndpointT&, uint16_t);
    void operator()(lwip::events::discon_t, BaseEndpointT&, lwip::discon_reason);
    void operator()(lwip::events::recv_t, BaseEndpointT&, lwip::buffer&&);

    void set_buffer_mode() {
        m_buffer_mode = true;
    }

    ALWAYS_INLINE
    bool disconnected() const {
        return m_discon;
    }

private:
    void attach();

    bool m_buffer_mode = false;
    tos::semaphore m_buffer_sem{0};
    lwip::buffer m_buffer;

    BaseEndpointT m_ep;
    tos::mutex m_busy;
    tos::semaphore m_write_sync{0};
    bool m_discon{false};
    uint32_t m_sent_bytes = 0;
    uint32_t m_queued_bytes = 0;

    condition_variable m_write_cv;
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
    //    LOG_TRACE("Sent:", len);
    m_sent_bytes += len;
    m_write_sync.up();
    m_write_cv.notify_all();
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
tos::Task<int> tcp_stream<BaseEndpointT>::async_send(tos::span<const uint8_t> buf) {
    if (m_discon) {
        co_return 0;
    }
    auto to_send = co_await m_ep.async_send(buf);
    if (to_send <= 0) {
        co_return to_send;
    }
    m_queued_bytes += to_send;
    co_return m_queued_bytes;
}

template<class BaseEndpointT>
tos::Task<int> tcp_stream<BaseEndpointT>::async_write(tos::span<const uint8_t> buf) {
    auto sz = buf.size();
    while (!buf.empty()) {
        co_await m_busy;
        tos::unique_lock lk{m_busy, tos::adopt_lock};
        auto buffer_space = m_ep.available_send_buffer();
        LOG(buffer_space, buf.size());
        auto sending = buf.slice(0, std::min<size_t>(buf.size(), buffer_space));
        auto to_send = co_await async_send(sending);
        if (to_send <= 0) {
            co_await m_write_cv.async_wait(m_busy);
            continue;
        }

        buf = buf.slice(sending.size());

#ifdef TOS_TCP_VERBOSE
        tos_debug_print("sending: %d\n", int(to_send));
#endif
        while (m_sent_bytes < to_send && !m_discon) {
            co_await m_write_cv.async_wait(m_busy);
        }
#ifdef TOS_TCP_VERBOSE
        tos_debug_print("sentt: %d\n", int(m_sent_bytes));
#endif
    }
    co_return sz;
}

template<class BaseEndpointT>
int tcp_stream<BaseEndpointT>::write(tos::span<const uint8_t> buf) {
    tos::lock_guard lk{m_busy};
    if (m_discon) {
        return 0;
    }
    auto to_send = m_ep.send(buf) + m_sent_bytes;
    if (to_send != buf.size()) {
        return 0;
    }
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("sending: %d\n", int(to_send));
#endif
    while (m_sent_bytes != to_send && !m_discon) {
        m_write_sync.down();
    }
#ifdef TOS_TCP_VERBOSE
    tos_debug_print("sentt: %d\n", int(m_sent_bytes));
#endif
    return buf.size();
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
    if (m_buffer_mode) {
        m_buffer_sem.up();
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

template<class BaseEndpointT>
tos::Task<expected<lwip::buffer, read_error>>
tcp_stream<BaseEndpointT>::async_read_buffer() {
    if (m_discon && m_buffer.size() == 0) {
        co_return unexpected(read_error::disconnected);
    }

    co_await m_busy;
    tos::unique_lock lk{m_busy, tos::adopt_lock};
    co_await m_buffer_sem;

    co_return m_buffer.pop_front();
}
} // namespace tos