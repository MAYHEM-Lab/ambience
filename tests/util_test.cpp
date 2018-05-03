//
// Created by fatih on 5/3/18.
//

#include "catch.hpp"

#include <tos/ring_buf.hpp>

TEST_CASE("base ring buffer", "[base_ring_buf]")
{
    tos::base_ring_buf rb{ 4 };
    REQUIRE(rb.capacity() == 4);
    REQUIRE(rb.push() == 0);    REQUIRE(rb.size() == 1);
    REQUIRE(rb.push() == 1);    REQUIRE(rb.size() == 2);
    REQUIRE(rb.push() == 2);    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 3);    REQUIRE(rb.size() == 4);
    rb.pop();                   REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 0);    REQUIRE(rb.size() == 4);
    rb.pop();                   REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 1);    REQUIRE(rb.size() == 4);
    rb.pop();                   REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 2);    REQUIRE(rb.size() == 4);
    rb.pop();                   REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 3);    REQUIRE(rb.size() == 4);
    rb.pop();                   REQUIRE(rb.size() == 3);
    rb.pop();                   REQUIRE(rb.size() == 2);
    rb.pop();                   REQUIRE(rb.size() == 1);
    rb.pop();                   REQUIRE(rb.size() == 0);
    REQUIRE(rb.push() == 0);
    REQUIRE(rb.push() == 1);
    REQUIRE(rb.push() == 2);
    REQUIRE(rb.push() == 3);
}
