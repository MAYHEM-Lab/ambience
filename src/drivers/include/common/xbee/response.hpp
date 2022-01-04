//
// Created by fatih on 10/10/18.
//

#pragma once

#include "constants.hpp"
#include "types.hpp"
#include <boost/sml.hpp>
#include <chrono>
#include <stddef.h>
#include <tos/expected.hpp>
#include <tos/span.hpp>
#include <cstring>

namespace tos {
namespace xbee {
template<api_ids id>
struct response_base {
    static constexpr auto api_id = id;
};

struct modem_status : response_base<api_ids::MODEM_STATUS_RESPONSE> {
    enum class statuses : uint8_t
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

struct tx_status : response_base<api_ids::TX_STATUS_RESPONSE> {
    enum class statuses : uint8_t
    {
        success = 0,
        no_ack = 1,
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

enum class parser_state
{
    null,
    started,
    got_len_msb,
    got_len_lsb,
    got_api_id
};

namespace events {
struct byte {
    uint8_t byte;
};
struct pl_done {};
} // namespace events

inline void consume_payload(tx_status& ts, tos::span<const uint8_t> data, size_t index) {
    if (index == 0 && data.size() == 1) {
        return;
    } else if (index == 0 && data.size() == 2) {
        ts.status = tx_status::statuses(data[1]);
    } else if (index == 1) {
        ts.status = tx_status::statuses(data[0]);
    }
}

inline void
consume_payload(modem_status& ts, tos::span<const uint8_t> data, size_t index) {
    ts.status = modem_status::statuses(data[0]);
}

struct base_parse_state {
    uint8_t msb;
    uint8_t lsb;
};

template<class ResType>
struct parse_state : base_parse_state {
    static constexpr auto api_id = ResType::api_id;
    ResType res;
};

auto is_start = [](const events::byte& e) -> bool { return e.byte == START_BYTE; };

template<class ResType>
struct is_type_correct_t {
    template<class EvT>
    constexpr bool operator()(const EvT& ev) const {
        return api_ids(ev.byte) == ResType::api_id;
    }
};

template<class T>
struct store_msb {
    constexpr void operator()(events::byte ev, parse_state<T>& state) const {
        state.msb = ev.byte;
    }
};

template<class T>
struct store_lsb {
    constexpr void operator()(events::byte ev, parse_state<T>& state) const {
        state.lsb = ev.byte;
    }
};

template<class ResType>
struct xbee_response {
    auto operator()() const {
        using namespace boost::sml;
        namespace s = boost::sml;
        return make_transition_table(
            *"null"_s + s::event<events::byte>[is_start] = "got_start"_s,
            "got_start"_s + s::event<events::byte> / store_msb<ResType>{} = "got_msb"_s,
            "got_msb"_s + s::event<events::byte> / store_lsb<ResType>{} = "got_len"_s,
            "got_len"_s + s::event<events::byte>[is_type_correct_t<ResType>{}] =
                "api_id"_s,
            "got_len"_s + s::event<events::byte>[!is_type_correct_t<ResType>{}] =
                "null"_s,
            "api_id"_s + s::event<events::pl_done> = "got_payload"_s,
            "got_payload"_s + s::event<events::byte> = X);
    }
};

template<class Type>
struct simple_alloc {
    using obj_type = Type;

    Type operator()(api_ids id, length_t len) {
        return Type{};
    }
};

struct recv16 : response_base<api_ids::RX_16_RESPONSE> {
    recv16() = default;
    explicit recv16(int len)
        : m_raw_data(len) {
    }

    std::vector<uint8_t> m_raw_data;

    addr_16 from() const {
        uint8_t buf[2];
        memcpy(&buf, m_raw_data.data(), 2);
        std::swap(buf[0], buf[1]);
        addr_16 res;
        memcpy(&res, buf, 2);
        return res;
    }

    span<const uint8_t> data() const {
        return span<const uint8_t>(m_raw_data).slice(4);
    }
};

inline void consume_payload(recv16& ts, tos::span<const uint8_t> data, size_t index) {
    ts.m_raw_data[index] = data[0];
}

template<class Type>
struct recv_alloc {
    using obj_type = Type;

    Type operator()(api_ids id, length_t len) {
        return Type{int(len.len)};
    }
};

template<class ResType, class ResAllocator = simple_alloc<ResType>>
class sm_response_parser : ResAllocator {
public:
    template<class ResAllocatorU = ResAllocator>
    sm_response_parser(ResAllocatorU&& res = {})
        : ResAllocator(std::forward<ResAllocatorU>(res)) {
    }

    void consume(uint8_t data) {
        if (m_escape) {
            data ^= 0x20;
            m_escape = false;
        } else if (data == ESCAPE_BYTE) {
            m_escape = true;
            return;
        }
        using namespace boost::sml;

        if (sm.is("api_id"_s)) {
            consume_payload(m_pl, {&data, 1}, m_pl_index - 1);
            m_pl_index++;

            if (m_pl_index < get_len().len) {
                return;
            }

            sm.process_event(events::pl_done{});
            return;
        }

        sm.process_event(events::byte{data});

        if (sm.is("got_len"_s)) {
            // -1 excludes the api id byte as we have already consumed that
            m_pl = static_cast<ResAllocator>(*this)(
                get_api_id(), length_t{uint16_t(get_len().len - 1)});
            // confirm size
        }
    }

    length_t get_len() const {
        return length_t{(uint16_t)((uint16_t)m_s.msb << 8 | m_s.lsb)};
    }
    api_ids get_api_id() const {
        return m_s.api_id;
    }

    bool finished() const {
        using namespace boost::sml;
        return sm.is(X);
    }

    typename ResAllocator::obj_type&& get_payload() && {
        return std::move(m_pl);
    }

    std::string_view state() const {
        std::string_view res;
        sm.visit_current_states([&](auto&& state) { res = state.c_str(); });
        return res;
    }

private:
    bool m_escape{false};

    uint8_t m_pl_index = 1;
    typename ResAllocator::obj_type m_pl;

    parse_state<ResType> m_s;
    boost::sml::sm<xbee_response<ResType>> sm{m_s};
};

class response_parser {
public:
    parse_errors consume(uint8_t data);

    length_t get_len() const;

    api_ids get_api_id() const {
        return m_id;
    }

private:
    bool m_escaped = false;
    parser_state m_state = parser_state::null;

    uint8_t m_len_msb, m_len_lsb;
    api_ids m_id;
};

enum class xbee_errors
{
    xbee_non_responsive
};

template<class StreamT, class AlarmT>
tos::expected<xbee::modem_status, xbee_errors> read_modem_status(StreamT& str,
                                                                 AlarmT& alarm) {
    using namespace std::chrono_literals;
    xbee::sm_response_parser<xbee::modem_status> parser({});
    while (!parser.finished()) {
        std::array<uint8_t, 1> rbuf;
        auto r = str.read(rbuf, alarm, 5s);
        if (r.size() != 1) {
            // xbee isn't waking up
            return unexpected(xbee_errors::xbee_non_responsive);
        }
        parser.consume(rbuf[0]);
    }
    return std::move(parser).get_payload();
}

template<class StreamT, class AlarmT>
tos::expected<xbee::tx_status, xbee_errors> read_tx_status(StreamT& str, AlarmT& alarm) {
    using namespace std::chrono_literals;
    xbee::sm_response_parser<xbee::tx_status> parser({});
    while (!parser.finished()) {
        std::array<uint8_t, 1> rbuf;
        auto r = str.read(rbuf, alarm, 100ms);
        if (r.size() != 1) {
            // xbee is non responsive
            return unexpected(xbee_errors::xbee_non_responsive);
        }
        parser.consume(rbuf[0]);
    }
    return std::move(parser).get_payload();
}

template<class StreamT, class AlarmT>
tos::expected<xbee::recv16, xbee_errors> receive(StreamT& str, AlarmT& alarm) {
    using namespace std::chrono_literals;
    xbee::sm_response_parser<tos::xbee::recv16, tos::xbee::recv_alloc<tos::xbee::recv16>>
        parser({});
    while (!parser.finished()) {
        std::array<uint8_t, 1> rbuf;
        auto r = str.read(rbuf);
        if (r.size() != 1) {
            // xbee is non responsive
            return unexpected(xbee_errors::xbee_non_responsive);
        }
        parser.consume(rbuf[0]);
    }
    return std::move(parser).get_payload();
}

template<class StreamT, class AlarmT>
tos::expected<xbee::recv16, xbee_errors>
receive(StreamT& str, AlarmT& alarm, std::chrono::milliseconds ms) {
    using namespace std::chrono_literals;
    xbee::sm_response_parser<tos::xbee::recv16, tos::xbee::recv_alloc<tos::xbee::recv16>>
        parser({});
    while (!parser.finished()) {
        std::array<uint8_t, 1> rbuf;
        auto r = str.read(rbuf, alarm, ms);
        if (r.size() != 1) {
            // xbee is non responsive
            return unexpected(xbee_errors::xbee_non_responsive);
        }
        parser.consume(rbuf[0]);
    }
    return std::move(parser).get_payload();
}
} // namespace xbee
} // namespace tos

// IMPL

namespace tos {
namespace xbee {
inline parse_errors response_parser::consume(uint8_t data) {
    if (m_escaped) {
        data ^= 0x20;
        m_escaped = false;
    }

    if (data == ESCAPE_BYTE) {
        if (m_state == parser_state::null) {
            return parse_errors::NONE;
        }
        m_escaped = true;
        return parse_errors::NONE;
    }

    if (data == START_BYTE && m_state != parser_state::null) {
        return parse_errors::UNEXPECTED_START_BYTE;
    }

    switch (m_state) {
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

inline length_t response_parser::get_len() const {
    if ((int)m_state < (int)parser_state::got_len_lsb) {
        return {0xFFFF};
    }

    return length_t{(uint16_t)((uint16_t)m_len_msb << 8 | m_len_lsb)};
}
} // namespace xbee

} // namespace tos