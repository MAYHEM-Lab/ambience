//
// Created by fatih on 7/14/19.
//

#include "doctest.h"

#include <iostream>
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

    for (int i = -10'000; i <= 10'000; ++i) {
        auto our_itoa = tos::itoa(i);
        auto std_itoa = std::to_string(i);
        std::string conv = std::string(our_itoa.begin(), our_itoa.end());
        REQUIRE_EQ(std_itoa, conv);
    }
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

TEST_CASE("float test") {
    std::array<char, 1024> buf;
    tos::omemory_stream str(buf);
    tos::print(str, (double)15.25);

    using namespace std::string_view_literals;
    REQUIRE_EQ("15.25000000"sv,
               std::string_view((const char*)str.get().data(), str.get().size()));
}

} // namespace
} // namespace tos