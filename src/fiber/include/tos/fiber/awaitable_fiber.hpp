#pragma once

#include <tos/fiber.hpp>
#include <tos/fiber/basic_fiber.hpp>
#include <tos/fiber/this_fiber.hpp>
#include <tos/late_constructed.hpp>
#include <type_traits>

namespace tos::fiber {
template<class FnT>
auto async(FnT&& fn) {
    using return_type = std::remove_cvref_t<decltype(fn())>;
    constexpr bool is_void = std::is_same_v<return_type, void>;

    struct dummy {};

    struct typed_future : private std::conditional_t<is_void, dummy, late_constructed<return_type>> {
        explicit typed_future(FnT&& fn) {
            m_fib = owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, [this, fn = std::forward<FnT>(fn)](auto& fib) {
                if constexpr (is_void) {
                    fn();
                } else {
                    this->emplace_fn(fn);
                }
                auto old_state = std::exchange(m_state, done);
                if (old_state == initial_run) {
                    return;
                }
                swap_fibers(*m_fib, *m_prev);
            });
            m_fib->resume();
        }

        return_type get(any_fiber& caller = *current_fiber()) requires (!is_void) {
            wait(caller);
            return std::move(static_cast<late_constructed<return_type>*>(this)->get());
        }

        void wait(any_fiber& caller = *current_fiber()) {
            if (m_state == done) {
                return;
            }
            m_state = waiting;
            m_prev = &caller;
            swap_fibers(*m_prev, *m_fib);
        }

        enum state {
            initial_run,
            waiting,
            done
        };
        state m_state = initial_run;
        any_fiber* m_prev;
        any_fiber* m_fib;
    };

    return typed_future(std::forward<FnT>(fn));
};
} // namespace tos::fiber