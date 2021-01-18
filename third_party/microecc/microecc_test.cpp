//
// Created by fatih on 12/8/19.
//

#include <cstring>
#include <doctest.h>
#include <tos/debug/debug.hpp>
#include <uECC.h>

namespace {
TEST_CASE("session key gen works") {
    auto curve = uECC_secp160r1();

    uint8_t private1[21];
    uint8_t private2[21];

    uint8_t public1[40];
    uint8_t public2[40];

    uint8_t secret1[20];
    uint8_t secret2[20];

    REQUIRE_EQ(1, uECC_make_key(public1, private1, curve));
    REQUIRE_EQ(1, uECC_make_key(public2, private2, curve));
    REQUIRE_EQ(1, uECC_shared_secret(public2, private1, secret1, curve));
    REQUIRE_EQ(1, uECC_shared_secret(public1, private2, secret2, curve));
    REQUIRE_EQ(0, memcmp(secret1, secret2, 20));
}
}