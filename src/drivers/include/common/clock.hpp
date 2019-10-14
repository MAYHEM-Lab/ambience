//
// Created by fatih on 10/13/19.
//

#pragma once

#include <chrono>
#include <cstdint>
#include <utility>
#include <tos/function_ref.hpp>

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
class clock : public non_copy_movable {
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
};
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
    auto fractional_part = m_timer->get_counter() * 1000 * m_period / m_timer->get_period();
    return time_point(duration(static_cast<uint64_t>(m_ticks) * 1000 * m_period + fractional_part));
}

template<class TimerT>
clock<TimerT>::clock(TimerT timer) : m_timer{std::move(timer)} {
    m_timer->set_frequency(50);
    m_period = 1000 / 50; // in milliseconds
    m_timer->set_callback(function_ref<void()>(*this));
    m_timer->enable();
}

} // namespace tos