//
// Created by fatih on 10/10/18.
//

#pragma once

#include "types.hpp"
#include "constants.hpp"
#include <stddef.h>
#include <tos/expected.hpp>

namespace tos
{
namespace xbee
{
    struct response
    {
    public:
    private:
        length_t m_len;
        api_ids m_api_id;
    };

    enum class parser_state
    {
        null,
        started,
        got_len_msb,
        got_len_lsb,
        got_api_id
    };

    class response_parser
    {
    public:

        parse_errors consume(uint8_t data);

        response get() const;

        length_t get_len() const;

        api_ids get_api_id() const { return m_id; }

    private:

        bool m_escaped = false;
        parser_state m_state = parser_state::null;
        size_t m_loc = 0;
        uint8_t m_len_msb, m_len_lsb;
        api_ids m_id;
    };
}
}

// IMPL

namespace tos
{
    namespace xbee
    {
        parse_errors response_parser::consume(uint8_t data) {
            if (m_escaped)
            {
                data ^= 0x20;
                m_escaped = false;
            }

            if (data == ESCAPE_BYTE)
            {
                if (m_state == parser_state::null)
                {
                    return parse_errors::NONE;
                }
                m_escaped = true;
                return parse_errors::NONE;
            }

            if (data == START_BYTE && m_state != parser_state::null)
            {
                return parse_errors::UNEXPECTED_START_BYTE;
            }

            switch (m_state)
            {
                case parser_state::null:
                    if (data == START_BYTE)
                        m_state = parser_state::started;
                    return parse_errors::NONE;

                case parser_state::started:
                    m_len_msb = data;
                    m_state = parser_state::got_len_msb;
                    return parse_errors::NONE;

                case parser_state::got_len_msb:
                    m_len_lsb = data;
                    m_state = parser_state::got_len_lsb;
                    return parse_errors::NONE;

                case parser_state::got_len_lsb:
                    m_id = static_cast<api_ids>(data);
                    m_state = parser_state::got_api_id;
                    return parse_errors::NONE;

                case parser_state::got_api_id:
                    return parse_errors::NONE;
            }

            return parse_errors::CHECKSUM_FAILURE;
        }

        length_t response_parser::get_len() const
        {
            if ((int)m_state < (int)parser_state::got_len_lsb)
            {
                return {0xFFFF};
            }

            return length_t{ (uint16_t)((uint16_t)m_len_msb << 8 | m_len_lsb) };
        }
    }

}