//
// Created by fatih on 4/18/18.
//

#pragma once

#include <numeric>
#include <tos/debug/log.hpp>
#include <tos/devices.hpp>
#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>

namespace tos {
namespace devs {
template<int N>
using timer_t = dev<struct _timer_t, N>;
template<int N>
static constexpr timer_t<N> timer{};
} // namespace devs

namespace detail {
template<int Channels>
class timer_multiplexer_base {
public:
    struct multiplexed_timer_state {
        bool enabled = false;
        int m_freq;
        function_ref<void()> m_cb{[](void*) {}};

        int m_period;
        int m_remaining_ticks = 0;
    };

    multiplexed_timer_state& state(int n) {
        return m_timers[n];
    }

    const multiplexed_timer_state& state(int n) const {
        return m_timers[n];
    }

    void tick() {
        for (auto& tmr : m_timers) {
            if (!tmr.enabled) {
                continue;
            }

            if (--tmr.m_remaining_ticks <= 0) {
                tmr.m_cb();
                tmr.m_remaining_ticks = tmr.m_period;
            }
        }
    }

    int compute_base_freq() {
        auto lcm = 1;
        bool one_enabled = false;
        for (auto& tmr : m_timers) {
            if (tmr.enabled) {
                lcm = std::lcm(lcm, tmr.m_freq);
                one_enabled = true;
            }
        }
        if (!one_enabled) {
            return 0;
        }
        return lcm;
    }

    void change_base_freq(int current, int to) {
        for (auto& tmr : m_timers) {
            if (tmr.enabled) {
                tmr.m_remaining_ticks = (tmr.m_remaining_ticks * to) / current;
                tmr.m_period = (tmr.m_period * to) / current;
            }
        }
    }

private:
    multiplexed_timer_state m_timers[Channels];
};
} // namespace detail

template<class BaseTimer, int Channels = 2>
class timer_multiplexer : public detail::timer_multiplexer_base<Channels> {
public:
    template<class... Ts>
    explicit timer_multiplexer(Ts&&... tmr)
        : m_base_timer{std::forward<Ts>(tmr)...} {
        m_base_timer->set_callback(mem_function_ref<&timer_multiplexer::tick>(*this));
    }

    struct multiplexed_timer : self_pointing<multiplexed_timer> {
    public:
        void set_frequency(int freq) {
            m_mux->set_channel_frequency(m_channel, freq);
        }

        void set_callback(function_ref<void()> cb) {
            state().m_cb = cb;
        }

        void enable() {
            m_mux->enable_channel(m_channel);
        }

        void disable() {
            m_mux->disable_channel(m_channel);
        }

        /**
         * Gets the current value of the counter of the timer.
         * Together with get_period, these functions can be used to implement a clock.
         * @return the current value of the counter.
         */
        uint32_t get_counter() const {
            return state().m_period - state().m_remaining_ticks;
        }

        /**
         * Gets the period of the timer, i.e. the value of the counter at which the
         * tick callback is executed.
         * @return the period of the timer.
         */
        uint32_t get_period() const {
            return state().m_period;
        }

    private:
        auto& state() {
            return m_mux->state(m_channel);
        }

        auto& state() const {
            return m_mux->state(m_channel);
        }

        friend class timer_multiplexer;

        multiplexed_timer(timer_multiplexer& mux, int channel)
            : m_mux(&mux)
            , m_channel(channel) {
        }

        timer_multiplexer* m_mux;
        int m_channel;
    };

    void set_channel_frequency(int channel, int freq) {
        this->state(channel).m_freq = freq;

        if (this->state(channel).enabled) {
            update_freqs();
        }
    }

    void enable_channel(int channel) {
        // period = base frequency / own freq
        this->state(channel).enabled = true;
        update_freqs();

        this->state(channel).m_period = m_cur_freq / this->state(channel).m_freq;
        this->state(channel).m_remaining_ticks = this->state(channel).m_period;
    }

    void disable_channel(int channel) {
        this->state(channel).enabled = false;
        //        update_freqs();
    }

    multiplexed_timer channel(int n) {
        return multiplexed_timer(*this, n);
    }

    ~timer_multiplexer() {
        m_base_timer->disable();
    }

    int m_cur_freq = 0;
    BaseTimer m_base_timer;

private:
    void update_freqs() {
        auto new_freq = this->compute_base_freq();
        if (new_freq != m_cur_freq) {
            if (new_freq == 0) {
                m_base_timer->disable();
            } else {
                if (m_cur_freq != 0) {
                    this->change_base_freq(m_cur_freq, new_freq);
                }
                m_base_timer->disable();
                m_base_timer->set_frequency(new_freq);
                m_base_timer->enable();
            }
            m_cur_freq = new_freq;
        }
    }
};
} // namespace tos