//
// Created by fatih on 10/13/19.
//

#pragma once

#include <chrono>
#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/function_ref.hpp>
#include <tos/interrupt.hpp>
#include <tos/thread.hpp>
#include <utility>

namespace tos {
/**
 * This class template implements a continuous counter abstraction over a standard
 * tos timer.
 *
 * This type follows the tos principles of using object lifetimes to represent state.
 * To start the clock, construct one, and to stop it, destruct it.
 *
 * @tparam TimerT type of the timer to build the clock over.
 */
template<class TimerT>
class clock
    : public non_copy_movable
    , public self_pointing<clock<TimerT>> {
public:
    /**
     * Constructs and immediately starts the clock using the given timer.
     * @param timer timer to build the clock over.
     */
    explicit clock(TimerT timer);

    using rep = uint64_t;
    using period = std::micro;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<clock>;

    /**
     * Computes and returns the current time.
     * @return current time
     */
    time_point now() const;

    static const bool is_steady = true;

    ~clock();
    void operator()();

private:
    uint32_t m_period; // in milliseconds
    uint32_t m_ticks = 0;
    TimerT m_timer;
    mutable uint64_t m_last_now = 0;
};

struct any_steady_clock : self_pointing<any_steady_clock> {
    using rep = uint64_t;
    using period = std::micro;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<any_steady_clock>;

    static const bool is_steady = true;

    virtual time_point now() const = 0;

    virtual ~any_steady_clock() = default;
};

using any_clock = any_steady_clock;

namespace detail {
template<class ClockT>
class erased_clock : public any_steady_clock {
public:
    template<class... Ts>
    explicit erased_clock(Ts&&... clk)
        : m_impl{std::forward<Ts>(clk)...} {
    }

    time_point now() const override {
        return any_steady_clock::time_point{
            std::chrono::duration_cast<duration>(m_impl->now().time_since_epoch())};
    }

    ClockT m_impl;
};
} // namespace detail

template<class ClockT>
auto erase_clock(ClockT&& clock) -> detail::erased_clock<ClockT> {
    return detail::erased_clock<ClockT>{std::forward<ClockT>(clock)};
}

template<class ClockT>
void delay(const ClockT& clock, std::chrono::microseconds dur, bool yield) {
    delay_until(clock, clock.now() + dur, yield);
}

template<class ClockT>
void delay_until(const ClockT& clock, typename ClockT::time_point end, bool yield) {
    while (clock.now() < end) {
        if (yield) {
            tos::this_thread::yield();
        }
    }
}
} // namespace tos

namespace tos {
template<class TimerT>
clock<TimerT>::~clock() {
    m_timer->disable();
}

template<class TimerT>
void clock<TimerT>::operator()() {
    ++m_ticks;
}

template<class TimerT>
auto clock<TimerT>::now() const -> time_point {
    tos::int_guard ig;
    auto tick_part = static_cast<uint64_t>(m_ticks) * 1000 * m_period;

    const auto cnt = m_timer->get_counter();
    const auto period = m_timer->get_period();
    auto fractional_part = static_cast<uint64_t>(cnt) * 1000 * m_period / period;

    auto now = tick_part + fractional_part;

    if (now < m_last_now) {
        // we missed an interrupt!
        now += 1000 * m_period;
    }
    m_last_now = now;
    return time_point(duration(now));
}

template<class TimerT>
clock<TimerT>::clock(TimerT timer)
    : m_timer{std::move(timer)} {
    m_timer->set_frequency(1);
    m_period = 1000; // in milliseconds
    m_timer->set_callback(function_ref<void()>(*this));
    m_timer->enable();
}
} // namespace tos
