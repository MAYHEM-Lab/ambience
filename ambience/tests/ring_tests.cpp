#include <doctest.h>
#include <tos/ae/rings.hpp>

namespace tos::ae {
namespace {
TEST_CASE("Allocate wrapping works") {
    tos::ae::interface_storage<8> storage;
    auto iface = storage.make_interface();

    REQUIRE_EQ(0, iface.allocate_priv());
    REQUIRE_EQ(1, iface.allocate_priv());
    REQUIRE_EQ(2, iface.allocate_priv());
    REQUIRE_EQ(3, iface.allocate_priv());
    REQUIRE_EQ(4, iface.allocate_priv());
    REQUIRE_EQ(5, iface.allocate_priv());
    REQUIRE_EQ(6, iface.allocate_priv());
    REQUIRE_EQ(7, iface.allocate_priv());
    iface.release(0);
    REQUIRE_EQ(0, iface.allocate_priv());
}

TEST_CASE("Response queue is initialized correctly") {
    tos::ae::interface_storage<8> storage;
    auto iface = storage.make_interface();

    int last_seen = 0;
    REQUIRE_EQ(0, for_each(iface, *iface.host_to_guest, last_seen, [](auto& idx) {}));
}

TEST_CASE("User submission works") {
    tos::ae::interface_storage<8> storage;
    auto iface = storage.make_interface();

    submit_req<false>(iface, 1, 2, nullptr, nullptr);
    uint16_t req_last_seen = 0;

    req_last_seen =
        for_each(iface, *iface.guest_to_host, req_last_seen, [](ring_elem& el) {
            REQUIRE_EQ(tos::util::set_flag(elem_flag::req, elem_flag::in_use),
                       el.req.flags);
            REQUIRE_EQ(1, el.req.channel);
            REQUIRE_EQ(2, el.req.procid);
            REQUIRE_EQ(nullptr, el.req.arg_ptr);
            REQUIRE_EQ(nullptr, el.req.ret_ptr);
        });

    REQUIRE_EQ(1, req_last_seen);
}
} // namespace
} // namespace tos::ae