#pragma once

#include <tos/detail/coro.hpp>

namespace tos::ae {

struct req_elem {
    const void* arg_ptr;
    void* ret_ptr;
    uint8_t procid;
    uint8_t channel;
    void* user_ptr;

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
};

union ring_elem {
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

    uint16_t allocate() {
        return 0;
    }
};
} // namespace tos::ae