#pragma once

#include <cstdint>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/span.hpp>
#include <utility>

namespace tos::virtio {
enum queue_flags : uint16_t
{
    none = 0,
    next = 1 << 0,
    write = 1 << 1,
    indirect = 1 << 2,
    available = 1 << 7,
    used = 1 << 15
};

struct queue_descriptor {
    // Physical address
    uint64_t addr{};
    uint32_t len{};
    queue_flags flags{};
    uint16_t next{};
};

static_assert(sizeof(queue_descriptor) == 16);

struct queue_available {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[]; // stores ids
    //    uint16_t used_event;
};

static_assert(sizeof(queue_available) == 4);

struct queue_used_elem {
    uint32_t id;
    uint32_t len;
};

struct queue_used {
    void disable_irq() volatile {
        flags = 1;
    }

    void enable_irq() volatile {
        flags = 0;
    }

    uint16_t flags;
    uint16_t index;
    queue_used_elem ring[];
    // uint16_t avail_event;
};

struct queue {
    uint16_t size;

    queue_descriptor* descriptors_base;
    queue_available* available_base;
    volatile queue_used* used_base;

    uint16_t next_buffer = 0;

    uint16_t last_seen_used = 0;

    template<class FnT>
    void for_each_used(FnT&& fn) {
        used_base->disable_irq();
        if (last_seen_used > used_base->index) {
            // The used ring has wrapped around.
            for (; last_seen_used != 0; ++last_seen_used) {
                // Cast away the volatile
                fn(*const_cast<queue_used_elem*>(
                    &used_base->ring[last_seen_used % size]));
            }
        }
        for (; last_seen_used < used_base->index; ++last_seen_used) {
            // Cast away the volatile
            fn(*const_cast<queue_used_elem*>(&used_base->ring[last_seen_used % size]));
        }
        used_base->enable_irq();
    }

    void dump_descriptor(int index);

    void submit_available(int index) {
        //  dump_descriptor(index);
        available_base->ring[available_base->index++ % size] = index;
    }

    std::pair<int, queue_descriptor*> alloc() {
        // TODO: need to find a descriptor that's actually free
        auto idx = next_buffer++ % size;
        return {idx, &descriptors()[idx]};
    }

    tos::span<const queue_descriptor> descriptors() const {
        return {descriptors_base, size};
    }

    tos::span<queue_descriptor> descriptors() {
        return {descriptors_base, size};
    }

    queue(const queue&) = delete;
    queue(queue&&) = default;

    explicit queue(uint16_t sz, tos::physical_page_allocator& palloc);
};
} // namespace tos::virtio