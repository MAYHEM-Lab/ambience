//
// Created by fatih on 10/10/18.
//

#pragma once

#include "types.hpp"
#include "constants.hpp"
#include <stddef.h>
#include <tos/expected.hpp>
#include <variant.hpp>
#include <boost/sml.hpp>
#include <tos/span.hpp>

namespace tos
{
namespace xbee
{
    template <api_ids id>
    struct response_base
    {
        static constexpr auto api_id = id;

    };

    struct modem_status
            : response_base<api_ids::MODEM_STATUS_RESPONSE>
    {
        enum class status : uint8_t
        {
            hw_reset = 0,
            wdt_rest = 1,
            associated = 2,
            disassociated = 3,
            sync_lost = 4,
            coord_realign = 5,
            coord_started = 6
        } status;
    };

    struct tx_status
            : response_base<api_ids::TX_STATUS_RESPONSE>
    {
        enum class statuses : uint8_t
        {
            success = 0,
            cca_fail = 0x2,
            invalid_dest_ep_success = 0x15,
            network_ack_fail = 0x21,
            not_joined = 0x22,
            self_addressed = 0x23,
            addr_not_found = 0x24,
            no_route = 0x25,
            payload_too_large = 0x74
        } status;
    };


    using response = mpark::variant<mpark::monostate, modem_status, tx_status>;

    inline response make_response(api_ids id)
    {
        switch (id)
        {
            case modem_status::api_id: return modem_status{};
            case tx_status::api_id: return tx_status{};
        }
        return {};
    }

    enum class parser_state
    {
        null,
        started,
        got_len_msb,
        got_len_lsb,
        got_api_id
    };

    namespace events
    {
        struct byte { uint8_t byte; };
    }

    void consume(tx_status& ts, tos::span<const uint8_t> data)
    {
        if (data.size() == 2)
        {
            ts.status = tx_status::statuses(data[1]);
        }
    }

    struct base_parse_state
    {
        uint8_t msb;
        uint8_t lsb;
    };

    template <class ResType>
    struct parse_state : base_parse_state
    {
        static constexpr auto api_id = ResType::api_id;
        ResType res;
    };

    auto is_start = [](const events::byte& e) -> bool { return e.byte == START_BYTE; };

    template <class ResType>
    struct is_type_correct_t
    {
        template <class EvT>
        constexpr bool operator()(const EvT& ev) const
        {
            return api_ids(ev.byte) == ResType::api_id;
        }
    };

    template <class T>
    struct store_msb
    {
        constexpr void operator()(events::byte ev, parse_state<T>& state) const
        {
            state.msb = ev.byte;
        }
    };

    template <class T>
    struct store_lsb
    {
        constexpr void operator()(events::byte ev, parse_state<T>& state) const
        {
            state.lsb = ev.byte;
        }
    };

    template <class T>
    struct store_payload
    {
        constexpr void operator()(tos::span<const uint8_t> ev, parse_state<T>& state) const
        {
            consume(state.res, ev);
        }
    };

    template <class ResType>
    struct xbee_response
    {
        auto operator()() const {
            using namespace boost::sml;
            return make_transition_table(
                *"null"_s     + event<events::byte> [ is_start ]               = "got_start"_s,
                "got_start"_s + event<events::byte> / store_msb<ResType>{}     = "got_msb"_s,
                "got_msb"_s   + event<events::byte> / store_lsb<ResType>{}     = "got_lsb"_s,
                "got_lsb"_s   + event<events::byte> [ is_type_correct_t<ResType>{} ] = "api_id"_s,
                "api_id"_s    + event<tos::span<const uint8_t>> = "got_payload"_s,
                "got_payload"_s + event<events::byte> = X
            );
        }
    };

    template <class ResType>
    class sm_response_parser
    {
    public:
        void consume(uint8_t data)
        {
            if (m_escape)
            {
                data ^= 0x20;
                m_escape = false;
            }
            else if (data == ESCAPE_BYTE) {
                m_escape = true;
                return;
            }
            using namespace boost::sml;

            if (sm.is("api_id"_s))
            {
                m_pl_index++;

                if (m_pl_index < get_len().len)
                {
                    return;
                }

                uint8_t buf[2];
                sm.process_event(tos::span<const uint8_t>(buf));
                return;
            }

            sm.process_event(events::byte{data});
        }

        length_t get_len() const { return length_t{ (uint16_t)((uint16_t)m_s.msb << 8 | m_s.lsb) }; }
        api_ids get_api_id() const { return m_s.api_id; }

        bool finished() const {
            using namespace boost::sml;
            return sm.is(X);
        }

    private:
        bool m_escape {false};

        uint8_t m_pl_index = 1;

        parse_state<ResType> m_s;
        boost::sml::sm<xbee_response<ResType>> sm {m_s};
    };

    class response_parser
    {
    public:
        parse_errors consume(uint8_t data);

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