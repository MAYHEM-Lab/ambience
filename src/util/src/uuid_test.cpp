#include <doctest.h>
#include <tos/uuid.hpp>

namespace tos {
namespace {
TEST_CASE("uuid from_canonical_string works") {
    auto uuid = tos::uuid_from_canonical_string("00112233-4455-6677-8899-aabbccddeeff");
    REQUIRE_EQ(0x00, uuid.id_[0]);
    REQUIRE_EQ(0x11, uuid.id_[1]);
    REQUIRE_EQ(0x22, uuid.id_[2]);
    REQUIRE_EQ(0x33, uuid.id_[3]);
    REQUIRE_EQ(0x44, uuid.id_[4]);
    REQUIRE_EQ(0x55, uuid.id_[5]);
    REQUIRE_EQ(0x66, uuid.id_[6]);
    REQUIRE_EQ(0x77, uuid.id_[7]);
    REQUIRE_EQ(0x88, uuid.id_[8]);
    REQUIRE_EQ(0x99, uuid.id_[9]);
    REQUIRE_EQ(0xaa, uuid.id_[10]);
    REQUIRE_EQ(0xbb, uuid.id_[11]);
    REQUIRE_EQ(0xcc, uuid.id_[12]);
    REQUIRE_EQ(0xdd, uuid.id_[13]);
    REQUIRE_EQ(0xee, uuid.id_[14]);
    REQUIRE_EQ(0xff, uuid.id_[15]);
}
} // namespace
} // namespace tos