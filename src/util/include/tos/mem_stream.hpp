//
// Created by fatih on 8/22/18.
//

#pragma once

#include "span.hpp"

namespace tos
{
    class memory_stream
    {
    public:
        explicit memory_stream(tos::span<char> buf) : m_buffer{buf}, m_wr_it{m_buffer.begin()} {}

        size_t write(tos::span<const char> buf){
            auto buf_it = buf.begin();
            while (m_wr_it != m_buffer.end() && buf_it != buf.end())
            {
                *m_wr_it++ = *buf_it++;
            }
            return buf_it - buf.begin();
        }

        tos::span<const char> get() {
            return m_buffer.slice(0, m_wr_it - m_buffer.begin());
        }

    private:
        tos::span<char> m_buffer;
        char* m_wr_it;
    };
}