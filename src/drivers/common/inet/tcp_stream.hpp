//
// Created by fatih on 6/30/18.
//

#pragma once

#include <drivers/arch/lx106/tcp.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/event.hpp>
#include <tos/span.hpp>
#include <tos/expected.hpp>

namespace tos
{
    enum class read_error
    {
        disconnected,
        timeout
    };

    template <class BaseEndpointT>
    class tcp_stream
    {
    public:

        explicit tcp_stream(BaseEndpointT&& ep) ALWAYS_INLINE;

        void write(span<const char>) ALWAYS_INLINE;

        expected<span<char>, read_error> read(span<char>);

        void operator()(lwip::events::sent_t, BaseEndpointT&) ICACHE_FLASH_ATTR;
        void operator()(lwip::events::discon_t, BaseEndpointT&, lwip::discon_reason) ICACHE_FLASH_ATTR;
        void operator()(lwip::events::recv_t, BaseEndpointT&, lwip::buffer&&) ICACHE_FLASH_ATTR;

        bool ALWAYS_INLINE disconnected() const { return m_discon; }

    private:

        void attach();

        lwip::buffer m_buffer;

        BaseEndpointT m_ep;
        tos::mutex m_busy;
        tos::semaphore m_write_sync{0};
        tos::semaphore m_read_sync{0};
        bool m_discon{false};

        tos::span<char>::iterator m_it{};
        tos::span<char>::iterator m_end{};
    };

    template <class BaseEndpointT>
    inline tcp_stream<BaseEndpointT>::tcp_stream(BaseEndpointT &&ep)
            : m_ep(std::move(ep)) {
        attach();
    }

    template <class BaseEndpointT>
    inline void tcp_stream<BaseEndpointT>::attach() {
        m_ep.attach(*this);
    }

    template <class BaseEndpointT>
    inline void tcp_stream<BaseEndpointT>::operator()(tos::lwip::events::sent_t, BaseEndpointT &) {
        m_write_sync.up();
    }

    template <class BaseEndpointT>
    inline void tcp_stream<BaseEndpointT>::operator()(tos::lwip::events::discon_t, BaseEndpointT &, lwip::discon_reason) {
        m_read_sync.up();
        m_discon = true;
    }

    template <class BaseEndpointT>
    inline void tcp_stream<BaseEndpointT>::write(tos::span<const char> buf) {
        tos::lock_guard<tos::mutex> lk{ m_busy };
        m_ep.send(buf);
        m_write_sync.down();
    }

    template <class BaseEndpointT>
    inline void tcp_stream<BaseEndpointT>::operator()(lwip::events::recv_t, BaseEndpointT &, lwip::buffer&& buf) {
        if (m_buffer.has_more())
        {
            m_buffer.append(std::move(buf));
        }
        else
        {
            m_buffer = std::move(buf);
        }
        m_read_sync.up();
    }

    template <class BaseEndpointT>
    inline expected<span<char>, read_error> tcp_stream<BaseEndpointT>::read(tos::span<char> to) {
        if (m_discon && !m_buffer.has_more())
        {
            return unexpected(read_error::disconnected);
        }

        tos::lock_guard<tos::mutex> lk{ m_busy };

        return m_buffer.read(to);
    }
}