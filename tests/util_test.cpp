//
// Created by fatih on 5/3/18.
//

#include "doctest.h"

#include <tos/fixed_fifo.hpp>
#include <tos/ring_buf.hpp>

namespace tos {
namespace {
TEST_CASE("base ring buffer") {
    tos::ring_buf rb{4};
    REQUIRE(rb.capacity() == 4);
    REQUIRE(rb.push() == 0);
    REQUIRE(rb.size() == 1);
    REQUIRE(rb.push() == 1);
    REQUIRE(rb.size() == 2);
    REQUIRE(rb.push() == 2);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 3);
    REQUIRE(rb.size() == 4);
    REQUIRE(rb.pop() == 0);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 0);
    REQUIRE(rb.size() == 4);
    REQUIRE(rb.pop() == 1);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 1);
    REQUIRE(rb.size() == 4);
    REQUIRE(rb.pop() == 2);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 2);
    REQUIRE(rb.size() == 4);
    REQUIRE(rb.pop() == 3);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.push() == 3);
    REQUIRE(rb.size() == 4);
    REQUIRE(rb.pop() == 0);
    REQUIRE(rb.size() == 3);
    REQUIRE(rb.pop() == 1);
    REQUIRE(rb.size() == 2);
    REQUIRE(rb.pop() == 2);
    REQUIRE(rb.size() == 1);
    REQUIRE(rb.pop() == 3);
    REQUIRE(rb.size() == 0);
    REQUIRE(rb.push() == 0);
    REQUIRE(rb.push() == 1);
    REQUIRE(rb.push() == 2);
    REQUIRE(rb.push() == 3);
}

TEST_CASE("fifo test") {
    tos::basic_fixed_fifo<char, 4, tos::ring_buf> ff;
    ff.push('a');
    REQUIRE(ff.pop() == 'a');
    ff.push('b');
    REQUIRE(ff.pop() == 'b');
    ff.push('c');
    ff.push('d');
    REQUIRE(ff.pop() == 'c');
    REQUIRE(ff.pop() == 'd');
    ff.push('e');
    ff.push('f');
    ff.push('g');
    ff.push('h');
    REQUIRE(ff.pop() == 'e');
    REQUIRE(ff.pop() == 'f');
    REQUIRE(ff.pop() == 'g');
    REQUIRE(ff.pop() == 'h');
}
} // namespace
} // namespace tos