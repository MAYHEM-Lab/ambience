//
// Created by fatih on 10/10/18.
//

#pragma once

#include <stdint.h>

#include "types.hpp"

namespace tos
{
namespace xbee
{
    enum class api_ids : uint8_t
    {
        TX_64_REQUEST = 0x0,
        TX_16_REQUEST = 0x1,
        AT_COMMAND_REQUEST = 0x08,
        AT_COMMAND_QUEUE_REQUEST = 0x09,
        REMOTE_AT_REQUEST = 0x17,
        ZB_TX_REQUEST = 0x10,
        ZB_EXPLICIT_TX_REQUEST = 0x11,
        RX_64_RESPONSE = 0x80,
        RX_16_RESPONSE = 0x81,
        RX_64_IO_RESPONSE = 0x82,
        RX_16_IO_RESPONSE = 0x83,
        AT_RESPONSE = 0x88,
        TX_STATUS_RESPONSE = 0x89,
        MODEM_STATUS_RESPONSE = 0x8a,
        ZB_RX_RESPONSE = 0x90,
        ZB_EXPLICIT_RX_RESPONSE = 0x91,
        ZB_TX_STATUS_RESPONSE = 0x8b,
        ZB_IO_SAMPLE_RESPONSE = 0x92,
        ZB_IO_NODE_IDENTIFIER_RESPONSE = 0x95,
        AT_COMMAND_RESPONSE = 0x88,
        REMOTE_AT_COMMAND_RESPONSE = 0x97
    };

    static constexpr frame_id_t DEFAULT_FRAME_ID{1};
    static constexpr frame_id_t NO_RESPONSE_FRAME_ID{0};

    static constexpr uint8_t START_BYTE = 0x7e;
    static constexpr uint8_t ESCAPE_BYTE = 0x7d;

    enum class tx_options : uint8_t {
        ack = 0,
        disable_ack = 1,
        broadcast = 4
    };

    enum class parse_errors : uint8_t
    {
        NONE = 0,
        CHECKSUM_FAILURE = 1,
        PACKET_EXCEEDS_BYTE_ARRAY_LENGTH = 2,
        UNEXPECTED_START_BYTE = 3
    };
}
}