//
// Created by fatih on 10/10/18.
//

#pragma once

#include "types.hpp"
#include "constants.hpp"
#include <tos/span.hpp>

namespace tos
{
namespace xbee
{
    template <api_ids ApiId>
    struct req
    {
        static constexpr api_ids api_id = ApiId;
        frame_id_t m_frame_id;
    };

    template <class AddrT>
    class tx_req : req<api_ids::TX_16_REQUEST>
    {
    public:
        tx_req(const AddrT& to, span<const uint8_t> data)
                : req{ DEFAULT_FRAME_ID }
                , m_addr{ to }
                , m_opts{ tx_options::ack }
                , m_data{ data }
        {}

        const AddrT& get_addr() const { return m_addr; }
        const tx_options& get_options() const { return m_opts; }
        const span<const uint8_t>& get_payload() const { return m_data; }

    private:
        AddrT m_addr;
        tx_options m_opts;
        span<const uint8_t> m_data;
    };
}
}