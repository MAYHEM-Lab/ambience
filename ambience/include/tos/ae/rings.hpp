#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <tos/detail/coro.hpp>
#include <tos/flags.hpp>

namespace tos::ae {
enum class elem_flag : uint8_t
{
    none = 0,
    // The element is a request, as opposed to a response
    req = 1,
    // Hypervisor to user element
    incoming = 2,
    // This element is not free
    in_use = 4
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

template<class FnT>
uint16_t for_each(const ring& ring, uint16_t last_seen, size_t size, const FnT& fn) {
    // TODO: handle overflow
    for (; last_seen < ring.head_idx; ++last_seen) {
        auto idx = ring.elems[last_seen % size];

        fn(idx);
    }

    return last_seen;
}

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

template<class FnT>
uint16_t for_each(
    interface& iface, const ring& ring, uint16_t last_seen, size_t size, const FnT& fn) {
    // TODO: handle overflow
    for (; last_seen < ring.head_idx; ++last_seen) {
        auto idx = ring.elems[last_seen % size];

        fn(iface.elems[idx]);
    }

    return last_seen;
}

template<size_t N>
struct interface_storage {
    interface_storage() = default;

    ring_elem elems[N];
    uint8_t req_arr[sizeof(ring) + N * sizeof(uint16_t)];
    uint8_t res_arr[sizeof(ring) + N * sizeof(uint16_t)];

    interface make_interface() {
        return interface{N, elems, new (&req_arr) ring{}, new (&res_arr) ring{}};
    }
};

template<bool FromHypervisor>
req_elem& submit_req(interface& iface, int channel, int proc, const void* params, void* res) {
    auto el_idx = iface.allocate();
    auto& req_el = iface.elems[el_idx].req;

    req_el.flags = elem_flag::req;
    if constexpr (FromHypervisor) {
        req_el.flags = tos::util::set_flag(req_el.flags, elem_flag::incoming);
    }

    req_el.channel = channel;
    req_el.procid = proc;
    req_el.arg_ptr = params;
    req_el.ret_ptr = res;
    req_el.user_ptr = nullptr;

    if constexpr (FromHypervisor) {
        iface.res->elems[iface.res->head_idx++ % iface.size] = el_idx;
    } else {
        iface.req->elems[iface.req->head_idx++ % iface.size] = el_idx;
    }

    return req_el;
}

template<bool FromHypervisor>
void respond(interface& iface, ring_elem& el) {
    auto el_idx = std::distance(iface.elems, &el);

    auto& req = el.req;
    auto& res = el.res;

    res.user_ptr = req.user_ptr;
    res.flags = elem_flag::incoming;

    if constexpr (FromHypervisor) {
        iface.res->elems[iface.res->head_idx++ % iface.size] = el_idx;
    } else {
        iface.req->elems[iface.req->head_idx++ % iface.size] = el_idx;
    }
}

inline void ack(interface& iface, ring_elem& elem) {
    elem.common.flags = elem_flag::none;
}
} // namespace tos::ae