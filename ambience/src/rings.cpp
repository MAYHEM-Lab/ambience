#include <atomic>
#include <tos/ae/rings.hpp>

namespace tos::ae {
int interface::host_to_guest_queue_depth() const {
    return host_to_guest->size(size, res_last_seen);
}

int interface::guest_to_host_queue_depth(uint16_t last_seen) const {
    return guest_to_host->size(size, last_seen);
}

interface::interface(int size, ring_elem* elems, ring* guest_to_host, ring* host_to_guest)
    : size{size}
    , elems{elems}
    , guest_to_host{guest_to_host}
    , host_to_guest{host_to_guest} {
    for (int i = 0; i < size - 1; ++i) {
        elems[i].free.next_free = &elems[i + 1].free;
    }
    elems[size - 1].free.next_free = nullptr;
    free_head.store(&elems[0].free, std::memory_order_release);
}

NO_INLINE
int32_t interface::allocate_entry() {
    auto res = free_head.load(std::memory_order_acquire);
    do {
        if (res == nullptr) {
            // No space currently
            return -1;
        }
    } while (!free_head.compare_exchange_weak(
        res, res->next_free, std::memory_order_release, std::memory_order_acquire));

    int32_t idx = std::distance(elems, reinterpret_cast<ring_elem*>(res));
    return idx;
}

void interface::release(int idx) {
    elems[idx].common.flags = elem_flag::released;

    auto expect = free_head.load(std::memory_order_acquire);
    auto store = &elems[idx].free;

    do {
        store->next_free = expect;
    } while (!free_head.compare_exchange_weak(
        expect, store, std::memory_order_release, std::memory_order_acquire));
}

bool ring::overflown(uint16_t last_seen) const {
    return last_seen > head_idx.load(std::memory_order_acquire);
}

bool ring::empty(uint16_t last_seen) const {
    return last_seen == head_idx.load(std::memory_order_acquire);
}

ssize_t ring::size(int ring_size, uint16_t last_seen) const {
    auto res = head_idx.load(std::memory_order_acquire) - last_seen;
    if (!overflown(last_seen)) {
        return res;
    }
    return res + ring_size;
}

std::pair<req_elem&, int>
prepare_req(interface& iface, int channel, int proc, const void* params, void* res) {
    auto el_idx = iface.allocate_entry();
    if (el_idx < 0) {
        while (true)
            ;
    }
    auto& req_el = iface.elems[el_idx].req;

    req_el.flags = tos::util::set_flag(elem_flag::req, elem_flag::in_use);

    req_el.channel = channel;
    req_el.procid = proc;
    req_el.arg_ptr = params;
    req_el.ret_ptr = res;

    return {req_el, el_idx};
}
} // namespace tos::ae