#pragma once

#include <cstdint>
#include <string_view>
#include <tos/detail/coro.hpp>
#include <tos/task.hpp>

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

void proc_res_queue(interface& iface);

inline auto&
submit_req(interface& iface, int channel, int proc, const void* params, void* res) {
    auto el_idx = iface.allocate();
    auto& req_el = iface.elems[el_idx].req;
    req_el.channel = channel;
    req_el.procid = proc;
    req_el.arg_ptr = params;
    req_el.ret_ptr = res;
    iface.req->elems[iface.req->head_idx++ % iface.size] = el_idx;
    return req_el;
}

tos::Task<void> log_str(std::string_view sv);
} // namespace tos::ae