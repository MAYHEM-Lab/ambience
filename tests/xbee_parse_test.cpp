//
// Created by fatih on 10/10/18.
//

#include "doctest.h"

#include <common/driver_base.hpp>
#include <common/xbee/request.hpp>
#include <common/xbee/response.hpp>
#include <tos/span.hpp>

namespace tos::xbee {
namespace {
TEST_CASE("xbee req gen") {
    struct to_dev_t : tos::self_pointing<to_dev_t> {
        int write(tos::span<const uint8_t> buf) {
            for (auto c : buf) {
                res.push_back(c);
            }
            return int(buf.size());
        }

        std::vector<uint8_t> res;
    } to_dev;

    addr_16 to_addr{0x1234};
    uint8_t buf[] = {'h', 'i'};
    frame_id_t f_id{0x01};

    tx16_req req{to_addr, buf, f_id};

    write_to(to_dev, req);

    auto& r = to_dev.res;
    REQUIRE(r[0] == xbee::START_BYTE);
    REQUIRE(r[1] == 0); // length msb
    REQUIRE(r[2] == 7); // 2 bytes payload, 5 bytes metadata
    REQUIRE(r[3] == uint8_t(api_ids::TX_16_REQUEST)); // api id
    REQUIRE(r[4] == f_id.id);                               // frame id
    REQUIRE(r[5] == uint8_t(to_addr.addr >> 8));            // address
    REQUIRE(r[6] == uint8_t(to_addr.addr & 0xFF));          // ...
    REQUIRE(r[7] == uint8_t(tx_options::ack));        // options
    REQUIRE(r[8] == buf[0]);
    REQUIRE(r[9] == buf[1]);
    REQUIRE(r[10] == uint8_t(0xE6)); // checksum
}

TEST_CASE("xbee sm res parse") {
    sm_response_parser<tx_status> p;

    uint8_t full_packet[] = {
        0x7E, // start byte
        0x00, // length msb
        0x01, // length lsb

        0x89, // api id (TX_STATUS_RESPONSE)
        0x01, // Frame ID
        0x00, // Status

        // 0xad  // Checksum
    };

    for (auto b : full_packet) {
        p.consume(b);
    }

    REQUIRE(p.finished());

    REQUIRE(p.get_len().len == 1);
    REQUIRE(p.get_api_id() == api_ids::TX_STATUS_RESPONSE);
}

TEST_CASE("xbee res parse") {
    response_parser p;

    uint8_t full_packet[] = {
        0x7E, // start byte
        0x00, // length msb
        0x01, // length lsb

        0x89, // api id (TX_STATUS_RESPONSE)
        0x01, // Frame ID
        0x00, // Status

        0xad // Checksum
    };

    for (auto b : full_packet) {
        REQUIRE(p.consume(b) == parse_errors::NONE);
    }

    REQUIRE(p.get_len().len == 1);
    REQUIRE(p.get_api_id() == api_ids::TX_STATUS_RESPONSE);
}
}
}