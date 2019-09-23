//
// Created by fatih on 5/8/18.
//

#pragma once

#include <chrono>
#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/devices.hpp>
#include <tos/event.hpp>
#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>

namespace tos {
struct sleeper : list_node<sleeper> {
    explicit sleeper(uint16_t ticks, const function_ref<void()>& fun)
        : sleep_ticks(ticks)
        , m_fun(fun) {
    }

private:
    uint16_t sleep_ticks;
    function_ref<void()> m_fun;

    template<class T>
    friend class alarm;
};

/**
 * This class template implements software alarm functionality over a
 * single hardware timer.
 *
 * The alarm abstraction allows threads to block on timers, also called
 * sleeping.
 *
 * The basic feature of the alarm is that it multiplexes a single
 * hardware timer to support multiple timer events.
 *
 * @tparam T type of the base timer
 */
template<class T>
class alarm : public self_pointing<alarm<T>> {
public:
    using alarm_handle = intrusive_list<sleeper>::iterator_t;

    explicit alarm(T& t)
        : m_timer(&t) {
    }

    /**
     * The calling thread will be blocked for the given amount
     * of time.
     *
     * @param dur duration to block for
     */
    void sleep_for(std::chrono::milliseconds dur) {
        event ev;
        volatile bool b = false;
        auto fun = [&ev, &b] { ev.fire_isr(); b = true; };
        sleeper s{uint16_t(dur.count()), tos::function_ref<void()>(fun)};
        set_alarm(s);
        if (!b)
        {
            ev.wait();
        }
    }

    auto set_alarm(sleeper& s) -> alarm_handle {
        tos::int_guard no_int;

        /**
         * find the first sleeper who'll wake up before s
         */
        auto it = m_sleepers.begin();
        while (it != m_sleepers.end() && it->sleep_ticks < s.sleep_ticks) {
            s.sleep_ticks -= it->sleep_ticks;
            ++it;
        }
        if (it != m_sleepers.end()) {
            it->sleep_ticks -= s.sleep_ticks;
        } else {
            start();
        }
        return m_sleepers.insert(it, s);
    }

    void cancel(alarm_handle it) {
        tos::int_guard no_int;

        auto next = it;
        ++next;
        if (next != m_sleepers.end()) {
            next->sleep_ticks += it->sleep_ticks;
        }
        m_sleepers.erase(it);

        if (m_sleepers.empty()) {
            stop();
        }
    }

    std::chrono::milliseconds resolution() const {
        return std::chrono::milliseconds(m_period);
    }

private:
    void start() {
        (*m_timer)->set_callback(
            {[](void* data) { static_cast<alarm*>(data)->tick_handler(); }, this});
        (*m_timer)->set_frequency(1000 / m_period);
        (*m_timer)->enable();
    }

    void stop() {
        (*m_timer)->disable();
    }

    void tick_handler() {
        sleeper& front = m_sleepers.front();
        auto prev = front.sleep_ticks;
        front.sleep_ticks -= m_period;
        if (front.sleep_ticks == 0 || front.sleep_ticks > prev) // may underflow
        {
            m_sleepers.pop_front();
            front.m_fun();
            if (m_sleepers.empty()) {
                stop();
            }
        }
    }

    int m_period = 1; // in milliseconds
    intrusive_list<sleeper> m_sleepers;
    T* m_timer;
};

namespace devs {
using alarm_t = tos::dev<struct alarm_t_, 0>;
static constexpr alarm_t alarm{};
} // namespace devs

template<class T>
auto open_impl(devs::alarm_t, T& tmr) {
    return alarm<std::remove_reference_t<T>>{tmr};
}

/**
 * This type represents a type erased alarm type.
 *
 * Prefer using the concrete alarm objects over this.
 */
struct any_alarm {
    using alarm_handle = intrusive_list<sleeper>::iterator_t;

    /**
     * Blocks the calling thread for the given amount of time
     *
     * Calling this function from a non-thread context has
     * undefined behavior
     *
     * @param dur time to block the calling thread
     */
    virtual auto sleep_for(std::chrono::milliseconds dur) -> void = 0;

    /**
     * Sets an alarm for the given sleeper object.
     *
     * As the caller will not be blocked upon this call,
     * it's up to the caller to ensure the sleeper object
     * lives until the callback is called.
     *
     * If the sleeper goes out of scope before the alarm
     * expires, the behavior is undefined.
     *
     * To avoid such cases, cancel the alarm with the
     * handle returned from this function.
     *
     * @param s sleeper to set an alarm for
     * @return handle to the alarm for the given sleeper
     */
    virtual auto set_alarm(sleeper& s) -> alarm_handle = 0;

    /**
     * Cancels the alarm pointed by the given handle.
     * @param handle alarm to cancel
     */
    virtual auto cancel(alarm_handle handle) -> void = 0;

    /**
     * Returns the resolution of the alarm.
     *
     * This value represents the minimum duration the
     * alarm supports for sleepers.
     *
     * @return resolution of the alarm
     */
    virtual auto resolution() const -> std::chrono::milliseconds = 0;

    virtual ~any_alarm() = default;
};

namespace detail {
template<class T>
class erased_alarm : public any_alarm {
public:
    erased_alarm(T t)
        : m_base_alarm(std::move(t)) {
    }

    void sleep_for(std::chrono::milliseconds dur) override {
        m_base_alarm->sleep_for(dur);
    }

    alarm_handle set_alarm(sleeper& s) override {
        return m_base_alarm->set_alarm(s);
    }

    void cancel(alarm_handle s) override {
        m_base_alarm->cancel(s);
    }

    std::chrono::milliseconds resolution() const override {
        return m_base_alarm->resolution();
    }

private:
    T m_base_alarm;
};
} // namespace detail

template<class AlarmT>
std::unique_ptr<any_alarm> erase_alarm(AlarmT&& alarm) {
    return std::make_unique<detail::erased_alarm<AlarmT>>(std::forward<AlarmT>(alarm));
}
} // namespace tos