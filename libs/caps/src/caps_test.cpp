//
// Created by fatih on 11/1/19.
//

#include <caps/caps.hpp>
#include <caps/common.hpp>
#include <doctest.h>

namespace caps {
namespace {
struct capability_t {
    int x;
};

bool operator==(capability_t a, capability_t b) {
    return a.x == b.x;
}

TEST_CASE("mkcaps list variant works") {
    auto list = mkcaps({capability_t{0}, capability_t{1}});
    REQUIRE(list->span().size() == 2);
    REQUIRE_EQ(0, list->span()[0].x);
    REQUIRE_EQ(1, list->span()[1].x);
}

TEST_CASE("cap_list clone works") {
    auto list = mkcaps({capability_t{0}, capability_t{1}});
    auto cloned = clone(*list);
    REQUIRE_EQ(list->span(), cloned->span());
}
} // namespace
}