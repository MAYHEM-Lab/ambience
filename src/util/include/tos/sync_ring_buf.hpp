//
// Created by fatih on 6/28/18.
//

#include <tos/ring_buf.hpp>
#include <tos/semaphore.hpp>

namespace tos
{
    class sync_ring_buf : private ring_buf
    {
    public:
        explicit sync_ring_buf(size_t cap) : ring_buf{cap}, m_read{0}, m_put{int8_t(cap)} {}

        using ring_buf::size;
        using ring_buf::capacity;
        using ring_buf::translate;

        size_t push()
        {
            m_put.down();
            auto res = ring_buf::push();
            m_read.up();
            return res;
        }

        //TODO: return an optional<size_t> here
        size_t push_isr()
        {
            auto cont = try_down_isr(m_put);
            if (!cont) return -1;
            auto res = ring_buf::push();
            m_read.up_isr();
            return res;
        }

        size_t pop()
        {
            m_read.down();
            auto res = ring_buf::pop();
            m_put.up();
            return res;
        }

    private:
        tos::semaphore m_read, m_put;
    };
}