//
// Created by fatih on 5/8/18.
//

#pragma once

#include <alarm_generated.hpp>
#include <chrono>
#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/devices.hpp>
#include <tos/event.hpp>
#include <tos/function_ref.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/scheduler.hpp>
#include <tos/thread.hpp>

namespace tos {
struct sleeper : list_node<sleeper> {
    explicit sleeper(uint16_t ticks, const function_ref<void()>& fun)
        : sleep_ticks(ticks)
        , m_fun(fun) {
    }

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
    using sleeper_type = sleeper;
    using alarm_handle = intrusive_list<sleeper>::iterator_t;

    explicit alarm(T t)
        : m_timer(std::move(t)) {
    }

    /**
     * The calling thread will be blocked for the given amount
     * of time.
     *
     * @param dur duration to block for
     */
    [[deprecated("Use tos::this_thread::sleep_for")]] void
    sleep_for(std::chrono::milliseconds dur) {
        tos::this_thread::sleep_for(*this, dur);
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

    alarm_handle unsafe_find_handle(sleeper& s) {
        return m_sleepers.unsafe_find(s);
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

    uint16_t time_to_ticks(std::chrono::milliseconds time) const {
        return time.count();
    }
    
    int sleeper_count() const {
        return m_sleepers.size();
    }

    bool running = false;
private:
    void start() {
        (*m_timer)->set_callback(
            {[](void* data) { static_cast<alarm*>(data)->tick_handler(); }, this});
        (*m_timer)->set_frequency(1000 / m_period);
        (*m_timer)->enable();
        running = true;
    }

    void stop() {
        (*m_timer)->disable();
        running = false;
    }

    void tick_handler() {
        if (m_sleepers.empty()) {
            // The interrupt may become pending while interrupts are disabled and the
            // waiters become empty.
            return;
        }

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

public:
    const int m_period = 1; // in milliseconds
    intrusive_list<sleeper> m_sleepers;
    T m_timer;
};

/**
 * This type represents a type erased alarm type.
 *
 * Prefer using the concrete alarm objects over this.
 */
struct any_alarm {
    using sleeper_type = sleeper;
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

    /**
     * Converts the given chrono duration to the number of ticks of the alarm.
     */
    virtual uint16_t time_to_ticks(std::chrono::milliseconds time) const = 0;

    virtual alarm_handle unsafe_find_handle(sleeper& s) = 0;

    virtual ~any_alarm() = default;
};

namespace detail {
template<class T>
class erased_alarm final
    : public any_alarm
    , public self_pointing<erased_alarm<T>> {
public:
    template<class... Ts>
    erased_alarm(Ts&&... t)
        : m_base_alarm(std::forward<Ts>(t)...) {
    }

    void sleep_for(std::chrono::milliseconds dur) override {
        tos::this_thread::sleep_for(m_base_alarm, dur);
    }

    alarm_handle unsafe_find_handle(sleeper& s) override {
        return m_base_alarm->unsafe_find_handle(s);
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

    uint16_t time_to_ticks(std::chrono::milliseconds time) const override {
        return m_base_alarm->time_to_ticks(time);
    }

    T m_base_alarm;
};
} // namespace detail

template<class AlarmT>
std::unique_ptr<detail::erased_alarm<AlarmT>> erase_alarm(AlarmT&& alarm) {
    return std::make_unique<detail::erased_alarm<AlarmT>>(std::forward<AlarmT>(alarm));
}

template<class AlarmT, class Rep, class Period>
auto async_sleep_for(AlarmT& alarm, const std::chrono::duration<Rep, Period>& duration) {
    struct awaiter : job {
        awaiter(AlarmT& alarm, const std::chrono::duration<Rep, Period>& duration)
            : job(current_context())
            , m_alarm{alarm}
            , m_sleeper{alarm->time_to_ticks(duration),
                        mem_function_ref<&awaiter::tick_fn>(*this)} {
        }

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) {
            m_cont = handle;
            m_alarm->set_alarm(m_sleeper);
        }

        void await_resume() {
        }

        void tick_fn() {
            tos::kern::make_runnable(*this);
        }

        void operator()() override {
            m_cont.resume();
        }

        using Type = std::remove_pointer_t<AlarmT>;

        AlarmT& m_alarm;
        typename Type::sleeper_type m_sleeper;
        std::coroutine_handle<> m_cont;
    };

    return awaiter{alarm, duration};
}

template<class BaseAlarm>
struct basic_async_alarm_impl : tos::ae::services::alarm::async_server {
    template<class... ArgTs>
    explicit basic_async_alarm_impl(ArgTs&&... args)
        : alarm(std::forward<ArgTs>(args)...) {
    }

    tos::Task<bool> sleep_for(tos::ae::services::milliseconds dur) override {
        co_await tos::async_sleep_for(alarm, std::chrono::milliseconds(dur.count()));
        co_return true;
    }

    BaseAlarm alarm;
};

template<class BaseAlarm>
struct basic_sync_alarm_impl : tos::ae::services::alarm::sync_server {
    template<class... ArgTs>
    explicit basic_sync_alarm_impl(ArgTs&&... args)
        : alarm(std::forward<ArgTs>(args)...) {
    }

    bool sleep_for(tos::ae::services::milliseconds dur) override {
        tos::this_thread::sleep_for(alarm, std::chrono::milliseconds(dur.count()));
        return true;
    }

    BaseAlarm alarm;
};

using async_any_alarm_impl = basic_async_alarm_impl<tos::any_alarm*>;
using sync_any_alarm_impl = basic_async_alarm_impl<tos::any_alarm*>;
} // namespace tos

namespace tos::this_thread {
template<class AlarmT, class Rep, class Period>
void sleep_for(AlarmT& alarm, const std::chrono::duration<Rep, Period>& duration) {
    event ev;
    volatile bool b = false;
    auto fun = [&ev, &b] {
        ev.fire_isr();
        b = true;
    };
    using Type = std::remove_pointer_t<AlarmT>;
    typename Type::sleeper_type s{meta::deref(alarm).time_to_ticks(duration),
                                  function_ref<void()>(fun)};
    meta::deref(alarm).set_alarm(s);
    if (!b) {
        ev.wait();
    }
}
} // namespace tos::this_thread