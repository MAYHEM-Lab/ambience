#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <new>
#include <tos/barrier.hpp>
#include <tos/compiler.hpp>
#include <tos/debug/log.hpp>
#include <tos/detail/coro.hpp>
#include <tos/fiber/basic_fiber.hpp>
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
    in_use = 4,
    released = 8,
};

struct interface;
struct req_elem {
    elem_flag flags;
    void* user_ptr;
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

        template<Fiber FibT>
        void fiber_suspend(FibT& fib);

        interface* iface;
        int id;
        req_elem* el;
        tos::function_ref<void()> ref{[](void*) {}};
    };

    template<bool FromHost>
    awaiter<FromHost> submit(interface* iface, int id, req_elem* el) {
        return {.iface = iface, .id = id, .el = el};
    }
};

struct elem {
    elem_flag flags;
    void* user_ptr;
};

struct res_elem {
    elem_flag flags;
    void* user_ptr;
};

struct free_elem {
    elem_flag flags;
    void* user_ptr;
    free_elem* next_free;
};

union ring_elem {
    elem common;
    req_elem req;
    res_elem res;
    free_elem free;

    ring_elem() {
    }
    
    explicit ring_elem(const ring_elem& el) {
        memcpy(this, &el, sizeof el);
    }
};

struct ring {
    std::atomic<uint16_t> head_idx;
    uint16_t elems[];

    bool overflown(uint16_t last_seen) const;
    bool empty(uint16_t last_seen) const;
    ssize_t size(int ring_size, uint16_t last_seen) const;

    void submit(uint16_t elem_idx, int sz) {
        elems[head_idx % sz] = elem_idx;
        head_idx.fetch_add(1, std::memory_order_release);
    }
};

struct interface {
    int size;

    ring_elem* elems;
    ring* guest_to_host;
    ring* host_to_guest;

    uint16_t res_last_seen = 0;

    // This member is modified both by the host and the guest.
    // Due to preemption, we have to use an atomic here.
    std::atomic<free_elem*> free_head = nullptr;

    int32_t allocate_entry();

    void release(int idx);

    int host_to_guest_queue_depth() const;
    int guest_to_host_queue_depth(uint16_t last_seen) const;

    interface(int size, ring_elem* elems, ring* guest_to_host, ring* host_to_guest);
};

template<class FnT>
uint16_t for_each(interface& iface, const ring& ring, uint16_t last_seen, const FnT& fn) {
    // The head index may change during our processing due to preemption.
    // We'll make a note of it and process only until that.
    auto head_backup = ring.head_idx.load(std::memory_order_acquire);

    if (last_seen > head_backup) {
        //        tos::debug::log("Wrapped around", &iface, &ring, last_seen,
        //        head_backup);
        // The used ring has wrapped around.
        for (; last_seen != 0; ++last_seen) {
            auto idx = ring.elems[last_seen % iface.size];
            //            tos::debug::log("Index", &iface, &ring, idx);
            ring_elem elem(iface.elems[idx]);
            iface.release(idx);

            fn(elem);
        }
    }

    for (; last_seen < head_backup; ++last_seen) {
        auto idx = ring.elems[last_seen % iface.size];
        //        tos::debug::log("Index", &iface, &ring, idx);
        ring_elem elem(iface.elems[idx]);
        iface.release(idx);

        fn(elem);
    }

    return last_seen;
}

std::pair<req_elem&, int>
prepare_req(interface& iface, int channel, int proc, const void* params, void* res);

template<bool FromHost>
void submit_elem(interface& iface, int el_idx) {
    if constexpr (FromHost) {
        iface.host_to_guest->submit(el_idx, iface.size);
    } else {
        iface.guest_to_host->submit(el_idx, iface.size);
    }
}

template<bool FromHost>
req_elem&
submit_req(interface& iface, int channel, int proc, const void* params, void* res) {
    const auto& [req_el, el_idx] =
        prepare_req(iface, channel, proc, params, res);

    submit_elem<FromHost>(iface, el_idx);

    return req_el;
}

template<bool FromHost>
void req_elem::awaiter<FromHost>::await_suspend(std::coroutine_handle<> handle) {
    ref = coro_resumer(handle);
    el->user_ptr = &ref;
    // tos::debug::log("Submitting", this->iface, el->user_ptr);
    submit_elem<FromHost>(*this->iface, this->id);
}

template<bool FromHost>
template<Fiber FibT>
void req_elem::awaiter<FromHost>::fiber_suspend(FibT& fib) {
    ref = fiber_resumer(fib);
    el->user_ptr = &ref;
    // tos::debug::log("Submitting", this->iface, el->user_ptr);
    submit_elem<FromHost>(*this->iface, this->id);
    fib.suspend();
}

template<bool FromHost>
void respond(interface& iface, void* user_ptr) {
    auto el_idx = iface.allocate_entry();
    if (el_idx < 0) {
        while (true)
            ;
    }

    auto& res = iface.elems[el_idx].res;

    res.user_ptr = user_ptr;
    res.flags = elem_flag::in_use;

    submit_elem<FromHost>(iface, el_idx);
}

template<size_t N>
struct interface_storage {
    interface_storage() = default;

    ring_elem elems[N]{};
    uint8_t req_arr[sizeof(ring) + N * sizeof(uint16_t)];
    uint8_t res_arr[sizeof(ring) + N * sizeof(uint16_t)];

    interface make_interface() {
        return interface {
            N, elems, new (&req_arr) ring{}, new (&res_arr) ring {
            }
        };
    }
};
} // namespace tos::ae