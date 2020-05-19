//
// Created by fatih on 12/7/19.
//

#pragma once

#include <common/clock.hpp>
#include <cstdint>
#include <tos/ft.hpp>
#include <tos/utility.hpp>

namespace tos::bench {
struct end_iterator {};

template<class ClockT>
struct state;

template<class ClockT>
struct state_iterator;

/**
 * Objects of this type are used to measure the duration of
 * a single iteration of the benchmarks.
 */
template<class ClockT>
struct iteration_sentinel : tos::non_copy_movable {
    explicit iteration_sentinel(state_iterator<ClockT>& state);

    ~iteration_sentinel();

    // Will report back to this state
    state_iterator<ClockT>* m_state;
    ClockT* m_clock;
    typename std::remove_pointer_t<ClockT>::time_point begin;
};

template<class ClockT>
struct state_iterator {
    void report_duration(typename std::remove_pointer_t<ClockT>::duration dur);

    iteration_sentinel<ClockT> operator*() {
        return iteration_sentinel<ClockT>{*this};
    };

    state_iterator& operator++() {
        return *this;
    }

    explicit state_iterator(state<ClockT>& state);

    state<ClockT>* m_state;
    ClockT* m_clock;
    bool m_finished = false;

    uint32_t m_iterations = 0;
    uint64_t m_sum = 0;
    uint64_t m_sq_sum = 0;

    uint64_t compute_mean() const {
        return m_sum / m_iterations;
    }

    uint64_t compute_variance() const {
        return (m_sq_sum - (m_sum * m_sum) / m_iterations) / (m_iterations - 1);
    }

    friend bool operator!=(const state_iterator& state, const struct end_iterator&) {
        return !state.m_finished;
    }
    friend bool operator!=(const struct end_iterator&, const state_iterator& state) {
        return !state.m_finished;
    }
    friend bool operator==(const state_iterator& state, const struct end_iterator&) {
        return state.m_finished;
    }
    friend bool operator==(const struct end_iterator&, const state_iterator& state) {
        return state.m_finished;
    }
};

template<class ClockT>
class state {
public:
    explicit state(ClockT& clock)
        : m_clock(&clock) {
    }

    state_iterator<ClockT> begin() {
        return state_iterator<ClockT>{*this};
    }

    end_iterator end() const {
        return {};
    }

    typename std::remove_pointer_t<ClockT>::duration mean;
    uint32_t iters;
    uint64_t sums;
    uint64_t squares;
    ClockT* m_clock;
};
} // namespace tos::bench

// impl

namespace tos::bench {
template<class ClockT>
state_iterator<ClockT>::state_iterator(state<ClockT>& state)
    : m_state{&state}
    , m_clock{state.m_clock} {
}

template<class ClockT>
void state_iterator<ClockT>::report_duration(
    typename std::remove_pointer_t<ClockT>::duration dur) {
    m_sum += dur.count();
    m_sq_sum += dur.count() * dur.count();
    ++m_iterations;
    if (m_iterations >= 100) {// && compute_variance() == 0 || m_iterations > 100) {
        m_state->mean = typename std::remove_pointer_t<ClockT>::duration{compute_mean()};
        m_state->iters = m_iterations;
        m_state->sums = m_sum;
        m_state->squares = m_sq_sum;
        m_finished = true;
    }
}

template<class ClockT>
iteration_sentinel<ClockT>::iteration_sentinel(state_iterator<ClockT>& state)
    : m_state{&state}
    , m_clock(m_state->m_clock)
    , begin{(*m_clock)->now()} {
    tos::detail::memory_barrier();
}

template<class ClockT>
iteration_sentinel<ClockT>::~iteration_sentinel() {
    tos::detail::memory_barrier();
    m_state->report_duration((*m_clock)->now() - begin);
    tos::this_thread::yield();
}

using any_state = state<any_clock*>;
} // namespace tos::bench