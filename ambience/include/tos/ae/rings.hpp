#pragma once

#include <tos/detail/coro.hpp>

namespace tos::ae {
enum class elem_flag : uint8_t {
    req = 1,
    incoming = 2
};

struct elem {
    void* user_ptr;
    elem_flag flags;
};

struct req_elem {
    void* user_ptr;
    elem_flag flags;
    uint8_t channel;
    uint8_t procid;
    const void* arg_ptr;
    void* ret_ptr;

    auto operator co_await() {
        struct awaiter {
            bool await_ready() const noexcept {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle) {
                el->user_ptr = handle.address();
            }

            void await_resume() const {
            }

            req_elem* el;
        };

        return awaiter{this};
    }
};

struct res_elem {
    void* user_ptr;
    elem_flag flags;
};

union ring_elem {
    elem common;
    req_elem req;
    res_elem res;
};

struct ring {
    uint16_t head_idx;
    uint16_t elems[];
};

struct interface {
    int size;

    ring_elem* elems;
    ring* req;
    ring* res;

    uint16_t res_last_seen = 0;
    int next_elem = 0;

    uint16_t allocate() {
        return next_elem++ % size;
    }
};
} // namespace tos::ae