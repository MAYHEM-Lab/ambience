//
// Created by fatih on 10/10/18.
//

#include "catch.hpp"
#include <drivers/common/xbee/response.hpp>

TEST_CASE("xbee res parse", "[tos-xbee]")
{
    namespace xbee = tos::xbee;
    xbee::response_parser p;

    REQUIRE(p.consume(xbee::ESCAPE_BYTE) == xbee::parse_errors::NONE);
    REQUIRE(p.consume(xbee::START_BYTE)  == xbee::parse_errors::NONE);


}