#include <common/timer.hpp>
#include <doctest.h>

namespace tos {
namespace {
TEST_CASE("timer multiplexer with 2 timers, frequency computation works") {
    detail::timer_multiplexer_base<2> base;
    base.state(0).enabled = true;
    base.state(1).enabled = true;

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 5;
    REQUIRE_EQ(10, base.compute_base_freq());

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 3;
    REQUIRE_EQ(30, base.compute_base_freq());

    base.state(0).m_freq = 100;
    base.state(1).m_freq = 8;
    REQUIRE_EQ(200, base.compute_base_freq());
}
TEST_CASE("timer multiplexer with 3 timers, frequency computation works") {
    detail::timer_multiplexer_base<3> base;
    base.state(0).enabled = true;
    base.state(1).enabled = true;
    base.state(2).enabled = true;

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 5;
    base.state(2).m_freq = 1;
    REQUIRE_EQ(10, base.compute_base_freq());

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 3;
    base.state(2).m_freq = 1;
    REQUIRE_EQ(30, base.compute_base_freq());

    base.state(0).m_freq = 100;
    base.state(1).m_freq = 8;
    base.state(2).m_freq = 3;
    REQUIRE_EQ(600, base.compute_base_freq());

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 5;
    base.state(2).m_freq = 15;
    REQUIRE_EQ(30, base.compute_base_freq());

    base.state(0).m_freq = 10;
    base.state(1).m_freq = 3;
    base.state(2).m_freq = 8;
    REQUIRE_EQ(120, base.compute_base_freq());

    base.state(0).m_freq = 100;
    base.state(1).m_freq = 8;
    base.state(2).m_freq = 1;
    REQUIRE_EQ(200, base.compute_base_freq());
}
} // namespace
} // namespace tos