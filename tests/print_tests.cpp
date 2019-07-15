//
// Created by fatih on 7/14/19.
//

#include "doctest.h"
#include <tos/print.hpp>
#include <tos/mem_stream.hpp>

TEST_CASE("itoa test")
{
    auto itoa_res = tos::itoa(0);

    REQUIRE_EQ(itoa_res.size(), 1);
    REQUIRE_EQ(itoa_res[0], '0');

    itoa_res = tos::itoa(-1);
    REQUIRE_EQ(itoa_res.size(), 2);
    REQUIRE_EQ(itoa_res[0], '-');
    REQUIRE_EQ(itoa_res[1], '1');
}
