#include <doctest.h>
#include <tos/intrusive_ptr.hpp>

namespace tos {
namespace {
struct intrusive_type {
    friend void intrusive_ref(intrusive_type* ptr) {
        ptr->ref++;
    }

    friend void intrusive_unref(intrusive_type* ptr) {
        ptr->ref--;
    }

    int ref = 0;
};

TEST_CASE("intrusive pointer works in a simple case") {
    intrusive_type elem;
    REQUIRE(elem.ref == 0);
    {
        intrusive_ptr<intrusive_type> ptr(&elem);
        REQUIRE(ptr.get() == &elem);
        REQUIRE(ptr->ref == 1);
    }
    REQUIRE(elem.ref == 0);
}

TEST_CASE("intrusive pointer copying works") {
    intrusive_type elem;
    {
        intrusive_ptr<intrusive_type> ptr(&elem);
        auto copy = ptr;
        REQUIRE(ptr.get() == copy.get());
        REQUIRE(ptr->ref == 2);
    }
    REQUIRE(elem.ref == 0);
}
} // namespace
} // namespace tos