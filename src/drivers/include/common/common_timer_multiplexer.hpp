#pragma once

#include <common/timer.hpp>
#include <common/alarm.hpp>
#include <common/clock.hpp>

namespace tos {
template<class BaseTimerT, size_t Count = 3>
struct common_timer_multiplex {
    using timer_mux_type = tos::timer_multiplexer<BaseTimerT, Count>;
    using channel_type = typename timer_mux_type::multiplexed_timer;
    using clock_type = tos::clock<channel_type>;
    using erased_clock_type = tos::detail::erased_clock<clock_type>;

    using alarm_type = tos::alarm<channel_type>;
    using erased_alarm_type = tos::detail::erased_alarm<alarm_type>;

    timer_mux_type tim_mux;
    channel_type timer{tim_mux.channel(0)};
    erased_clock_type clock{tim_mux.channel(1)};
    erased_alarm_type alarm{tim_mux.channel(2)};

    template<class... Ts>
    common_timer_multiplex(Ts&&... args)
        : tim_mux{std::forward<Ts>(args)...} {
    }
};
}