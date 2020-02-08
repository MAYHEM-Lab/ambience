//
// Created by fatih on 7/14/19.
//

#include "doctest.h"

#include <tos/mem_stream.hpp>
#include <tos/print.hpp>


namespace tos {
namespace {
TEST_CASE("itoa test") {
    auto itoa_res = tos::itoa(0);

    REQUIRE_EQ(itoa_res.size(), 1);
    REQUIRE_EQ(itoa_res[0], '0');

    itoa_res = tos::itoa(-1);
    REQUIRE_EQ(itoa_res.size(), 2);
    REQUIRE_EQ(itoa_res[0], '-');
    REQUIRE_EQ(itoa_res[1], '1');
}

TEST_CASE("print test") {
    std::array<char, 1024> buf;
    tos::omemory_stream str(buf);

    tos::print(str, "hello world");

    REQUIRE_EQ(str.get().size(), 11);
}

TEST_CASE("empty line") {
    std::array<char, 1024> buf;
    tos::omemory_stream str(buf);

    tos::println(str);

    REQUIRE_EQ(str.get().size(), 2);
}
} // namespace
} // namespace tos