#pragma once

#include <cstdint>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/span.hpp>
#include <utility>

namespace tos::virtio {
enum queue_flags : uint16_t
{
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

    std::pair<int, queue_descriptor*> alloc() {
        auto idx = next_buffer % size;
        ++next_buffer;
        return {idx, &descriptors()[idx]};
    }

    tos::span<const queue_descriptor> descriptors() const {
        return {descriptors_base, size};
    }

    tos::span<queue_descriptor> descriptors() {
        return {descriptors_base, size};
    }

    explicit queue(uint16_t sz, tos::physical_page_allocator& palloc);
};
} // namespace tos::virtio