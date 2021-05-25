#pragma once

#include <cstdint>
#include <tos/detail/coro.hpp>
#include <tos/intrusive_list.hpp>

namespace tos {
template<class T>
class async_init {
    static constexpr uintptr_t initialized = static_cast<uintptr_t>(-1);

    struct awaiter : tos::list_node<awaiter> {
        awaiter(async_init& self)
            : self{&self} {
        }

        bool await_ready() const noexcept {
            return self->has_value();
        }

        void await_suspend(std::coroutine_handle<> cont) {
            m_cont = cont;
            self->m_awaiters.push_back(*this);
        }

        T& await_resume() const noexcept {
            return self->value();
        }

        async_init* self;
        std::coroutine_handle<> m_cont;
    };

public:
    async_init()
        : m_awaiters{} {
    }

    ~async_init() {
    }

    awaiter operator co_await() {
        return awaiter{*this};
    }

    void set_value(T elem) {
        auto list = m_awaiters.shallow_copy();
        new (&m_init) initd{elem, initialized};
        for (auto& waiter : list) {
            waiter.m_cont.resume();
        }
    }

    bool has_value() const noexcept {
        return m_init.dummy == initialized;
    }

    T& value() {
        Assert(has_value());
        return m_init.m_t;
    }

private:
    struct [[gnu::packed]] initd {
        T m_t;
        uintptr_t dummy;
    };

    union {
        initd m_init;
        tos::intrusive_list<awaiter> m_awaiters;
    };
};
} // namespace tos