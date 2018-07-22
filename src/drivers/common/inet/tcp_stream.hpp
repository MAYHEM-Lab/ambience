//
// Created by fatih on 6/30/18.
//

#pragma once

#include <arch/lx106/tcp.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/event.hpp>
#include <tos/span.hpp>

namespace tos
{
    class tcp_stream
    {
    public:

        explicit tcp_stream(tos::esp82::tcp_endpoint&& ep);

        void write(span<const char>);

        span<char> read(span<char>);

        void operator()(esp82::events::sent_t, esp82::tcp_endpoint&);
        void operator()(esp82::events::discon_t, esp82::tcp_endpoint&);
        void operator()(esp82::events::recv_t, esp82::tcp_endpoint&, span<const char>);

    private:

        void attach();

        tos::fixed_fifo<char, 64> m_fifo;

        tos::esp82::tcp_endpoint m_ep;
        tos::mutex m_busy;
        tos::semaphore m_write_sync{0};
        tos::event m_read_sync;
        tos::semaphore m_disc{0};

        tos::span<char>::iterator m_it{};
        tos::span<char>::iterator m_end{};
    };

    inline tcp_stream::tcp_stream(tos::esp82::tcp_endpoint &&ep)
            : m_ep(std::move(ep)) {
        attach();
    }

    inline void tcp_stream::attach() {
        m_ep.attach(esp82::events::recv, *this);
        m_ep.attach(esp82::events::sent, *this);
    }

    inline void tcp_stream::operator()(tos::esp82::events::sent_t, tos::esp82::tcp_endpoint &) {
        m_write_sync.up();
    }

    inline void tcp_stream::write(tos::span<const char> buf) {
        tos::lock_guard<tos::mutex> lk{ m_busy };
        m_ep.send(buf);
        m_write_sync.down();
    }

    inline void tcp_stream::operator()(esp82::events::recv_t, esp82::tcp_endpoint &, span<const char> buf) {
        auto it = buf.begin();
        auto end = buf.end();

        while (it != end && m_it != m_end)
        {
            *m_it++ = *it++;
        }

        while (it != end && m_fifo.push_isr(*it++));

        m_read_sync.fire_isr();
    }

    inline span<char> tcp_stream::read(tos::span<char> to) {
        tos::lock_guard<tos::mutex> lk{ m_busy };

        {
            tos::int_guard ig;
            auto it = to.begin();
            auto end = to.end();

            while (m_fifo.size() > 0 && it != end)
            {
                *it++ = m_fifo.pop();
            }

            m_it = it;
            m_end = end;
        }

        if (m_it != m_end)
        {
            m_read_sync.wait();
        }

        return to.slice(0, m_it - to.begin());
    }
}