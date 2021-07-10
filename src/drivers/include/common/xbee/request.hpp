//
// Created by fatih on 10/10/18.
//

#pragma once

#include "constants.hpp"
#include "types.hpp"
#include "utility.hpp"

#include <common/driver_base.hpp>
#include <tos/print.hpp>
#include <tos/span.hpp>

namespace tos {
namespace xbee {
template<api_ids ApiId>
struct req {
    static constexpr api_ids api_id = ApiId;
    frame_id_t m_frame_id;
};

template<class AddrT>
class tx_req : public req<api_ids::TX_16_REQUEST> {
public:
    tx_req(const AddrT& to, span<const uint8_t> data, frame_id_t f_id = DEFAULT_FRAME_ID)
        : req{f_id}
        , m_addr{to}
        , m_opts{tx_options::ack}
        , m_data{data} {
    }

    constexpr const AddrT& get_addr() const {
        return m_addr;
    }
    constexpr const tx_options& get_options() const {
        return m_opts;
    }
    constexpr const span<const uint8_t>& get_payload() const {
        return m_data;
    }

    template<class StreamT>
    constexpr void put_to(StreamT& str) const {

        // remaining are payload bytes
        tos::print(str, uint8_t(api_id));
        tos::print(str, uint8_t(m_frame_id.id));
        write_addr(str, get_addr());
        tos::print(str, uint8_t(get_options()));
        span<const uint8_t> pl = get_payload();
        for (auto b : pl) {
            tos::print(str, b);
        }
        // tos::print(str, span<const char>(reinterpret_cast<const char*>(pl.data()),
        // pl.size()));
    }

private:
    AddrT m_addr;
    tx_options m_opts;
    span<const uint8_t> m_data;
};

template<class StreamT>
struct chk_str_t : self_pointing<chk_str_t<StreamT>> {
    StreamT str{};
    uint8_t chk_sum{0};

    constexpr int write(span<const uint8_t> buf) {
        auto res = meta::deref(str).write(buf);
        for (auto c : buf) {
            chk_sum += uint8_t(c);
        }
        return res;
    }
};

template<class StreamT, class ReqT>
constexpr void write_to(StreamT& str, const ReqT& req) {
    chk_str_t<StreamT*> chk_str{};
    chk_str.str = &str;

    tos::print(str, START_BYTE);
    tos::print(str, uint8_t(0)); // msb is always 0
    tos::print(str, uint8_t((req.get_payload().size() + 5) & 0xFF));

    req.put_to(chk_str);

    tos::print(str, uint8_t(uint8_t(0xFF) - chk_str.chk_sum));
}
} // namespace xbee
} // namespace tos