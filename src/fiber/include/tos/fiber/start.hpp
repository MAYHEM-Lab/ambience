#pragma once

#include "tos/stack_storage.hpp"
#include <tos/compiler.hpp>
#include <tos/fiber/basic_fiber.hpp>

namespace tos::fiber {
namespace detail {
auto setup_fiber_init_stack(span<uint8_t> stack, basic_fiber* fib_ptr) {
    auto ptr_pos = stack.end() - sizeof fib_ptr;
    memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
    ptr_pos = stack.end() - 2 * sizeof fib_ptr;
    memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
    ptr_pos = stack.end() - 3 * sizeof fib_ptr;
    memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
    ptr_pos = stack.end() - 4 * sizeof fib_ptr;
    memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
    ptr_pos = stack.end() - 5 * sizeof fib_ptr;
    memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);

    return std::pair(
        +[]() {
            basic_fiber* cur_fib;
            auto sp = cur_arch::get_stack_ptr();
            memcpy(&cur_fib, sp, sizeof cur_fib);
            // cur_arch::set_stack_ptr(static_cast<char*>(sp) + sizeof cur_fib);
            cur_fib->start();
            TOS_UNREACHABLE();
        },
        stack.end() - sizeof fib_ptr);
};

template<class FibT, class... Args>
auto setup_stack(span<uint8_t> stack, Args&&... args) {
    auto res = new (stack.end() - sizeof(FibT)) FibT{std::forward<Args>(args)..., stack};
    stack = stack.slice(0, stack.size() - sizeof(FibT));

    auto ctx_ptr = new (stack.end() - sizeof(processor_context)) processor_context;
    stack = stack.slice(0, stack.size() - sizeof(processor_context));

    auto [entry_point, sp] = detail::setup_fiber_init_stack(stack, res);

    start(*ctx_ptr, entry_point, sp);
    res->set_processor_state(*ctx_ptr);

    return res;
}
} // namespace detail

struct non_owning {
    template<class StartFn>
    struct fib : basic_fiber {
        NO_INLINE
        void start() {
            m_fn(*this);
        }

        fib(StartFn&& fn, span<uint8_t> stack)
            : m_fn{std::forward<StartFn>(fn)}
            , m_full_stack(stack) {
        }

        StartFn m_fn;
        span<uint8_t> m_full_stack;
    };

    template<class StartFn>
    static auto start(span<uint8_t> stack, StartFn&& fn) {
        return detail::setup_stack<fib<StartFn>>(stack, std::forward<StartFn>(fn));
    }
};

struct owning {
    template<class StartFn>
    struct fib : basic_fiber {
        NO_INLINE
        void start() {
            m_fn(*this);
        }

        fib(StartFn&& fn, span<uint8_t> stack)
            : m_fn{std::forward<StartFn>(fn)}
            , m_full_stack(stack) {
        }

        ~fib() {
            delete[] m_full_stack.data();
        }

        StartFn m_fn;
        span<uint8_t> m_full_stack;
    };

    template<class StartFn>
    static auto start(stack_size_t stack_sz, StartFn&& fn) {
        auto stack = new (std::nothrow) uint8_t[stack_sz.sz];
        if (!stack) {
            return nullptr;
        }
        return detail::setup_stack<fib<StartFn>>(span(stack, stack_sz.sz),
                                                 std::forward<StartFn>(fn));
    }
};
} // namespace tos::fiber
