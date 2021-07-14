#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <tos/detail/coro.hpp>
#include <tos/flags.hpp>
#include <tos/function_ref.hpp>

namespace tos::ae {
enum class elem_flag : uint8_t
{
    none = 0,
    // The element is a request, as opposed to a response
    req = 1,
    // The element has a continuation in its user_ptr instead
    next = 2,
    // This element is not free
    in_use = 4
};

struct elem {
    void* user_ptr;
    elem_flag flags;
};

struct interface;
struct req_elem {
    void* user_ptr;
    elem_flag flags;
    uint8_t channel;
    uint8_t procid;
    const void* arg_ptr;
    void* ret_ptr;

    template<bool FromHost>
    struct awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle);

        bool await_resume() const {
            return true;
        }

        interface* iface;
        int id;
        req_elem* el;
        tos::function_ref<void()> ref{[](void*) {}};
    };

    template <bool FromHost>
    awaiter<FromHost> submit(interface* iface, int id, req_elem* el) {
        return {
            .iface = iface,
            .id = id,
            .el = el
        };
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

    void submit(uint16_t elem_idx, int sz) {
        elems[head_idx++ % sz] = elem_idx;
    }
};

struct interface {
    int size;

    ring_elem* elems;
    ring* guest_to_host;
    ring* host_to_guest;

    uint16_t res_last_seen = 0;
    int next_elem = 0;

    uint16_t allocate() {
        return next_elem++ % size;
    }
};

template<class FnT>
uint16_t for_each(
    interface& iface, const ring& ring, uint16_t last_seen, size_t size, const FnT& fn) {
    if (last_seen > ring.head_idx) {
        // The used ring has wrapped around.
        for (; last_seen != 0; ++last_seen) {
            auto idx = ring.elems[last_seen % size];

            fn(iface.elems[idx]);
        }
    }

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

inline std::pair<req_elem&, int>
prepare_req(interface& iface, int channel, int proc, const void* params, void* res) {
    auto el_idx = iface.allocate();
    auto& req_el = iface.elems[el_idx].req;

    req_el.flags = elem_flag::req;

    req_el.channel = channel;
    req_el.procid = proc;
    req_el.arg_ptr = params;
    req_el.ret_ptr = res;
    req_el.user_ptr = nullptr;

    return {req_el, el_idx};
}

template<bool FromHypervisor>
void submit_elem(interface& iface, int el_idx) {
    if constexpr (FromHypervisor) {
        iface.host_to_guest->submit(el_idx, iface.size);
    } else {
        iface.guest_to_host->submit(el_idx, iface.size);
    }
}

template<bool FromHypervisor>
req_elem&
submit_req(interface& iface, int channel, int proc, const void* params, void* res) {
    const auto& [req_el, el_idx] = prepare_req(iface, channel, proc, params, res);

    submit_elem<FromHypervisor>(iface, el_idx);

    return req_el;
}

template<bool FromHost>
void req_elem::awaiter<FromHost>::await_suspend(std::coroutine_handle<> handle) {
    ref = coro_resumer(handle);
    el->user_ptr = &ref;
    submit_elem<FromHost>(*this->iface, this->id);
}

template<bool FromHypervisor>
void respond(interface& iface, ring_elem& el) {
    auto el_idx = std::distance(iface.elems, &el);

    auto& req = el.req;
    auto& res = el.res;

    res.user_ptr = req.user_ptr;
    res.flags = elem_flag::none;

    submit_elem<FromHypervisor>(iface, el_idx);
}
} // namespace tos::ae