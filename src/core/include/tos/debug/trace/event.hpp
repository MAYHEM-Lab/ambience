#pragma once

#include "tos/debug/log.hpp"
#include "tos/generator.hpp"
#include <memory>
#include <tos/debug/detail/logger_base.hpp>
#include <type_traits>

namespace tos::debug {
constexpr auto max_event_size = 32;

template<class T>
concept TraceEvent = std::is_pod<T>::value &&
                     sizeof(T) < max_event_size&& requires(const T& t) {
    log(t, std::declval<detail::any_logger&>());
};

namespace detail {
    struct model_vtbl {
        void (*log)(const void*, any_logger& logger);
    };

    template<TraceEvent T>
    constexpr model_vtbl vtbl_for() {
        return model_vtbl{.log = [](const void* ev, any_logger& logger) {
            log(*static_cast<const T*>(ev), logger);
        }};
    }

    // = Event vtable storage mechanism
    // While we could identify event vtables by pointers just like C++ vtables, a pointer
    // is 4 or 8 bytes wide. Since we will not have billions of event types, and we do not
    // support dynamic loading of event types, we can have all the vtables in an array,
    // and store an index into this array in our events. However, this could require us to
    // enumerate all possible event types in a single array. Instead, we let the linker
    // maintain this array for us by placing all these vtables in a special section in the
    // binary.
    template<TraceEvent T>
    [[gnu::section(".rodata.ev_vtbl")]] inline constexpr auto vtbl = vtbl_for<T>;

    extern const model_vtbl* ev_vtbl_start;
} // namespace detail

struct event_log {
    template<TraceEvent T>
    void publish(const T& event) {
        auto buf = m_alloc.allocate(sizeof(trace_event) + sizeof(event));
        new (buf) trace_event(event);
    }

    Generator<std::pair<const detail::model_vtbl*, const void*>> iter_events();

private:
    struct trace_event {
        template<TraceEvent T>
        trace_event(const T& ev) {
            m_evt_id = &detail::vtbl<T> - detail::ev_vtbl_start;
            new (m_data) T(ev);
        }

        uint16_t m_evt_id;
        uint8_t m_data[];
    };

    template<class SizeType>
    struct ring_allocator {
        void* allocate(int size, int align = 1) {
            auto alloc_sz = alloc_len(size, align);
            if (m_storage.size() < alloc_sz) {
                return nullptr;
            }

            if (std::distance(end, m_storage.end()) < alloc_sz) {
                // wrap-around
                // Must invalidate as many allocations as needed until we make enough
                // space.
            }

            auto sz = new (end) SizeType(alloc_sz);
            end = end + sz;
            return sz + 1;
        }

        int alloc_len(int size, int align) {
            return sizeof(SizeType) + size;
        }

        void free(void*) {
            // free is a no-op
        }

        Generator<std::pair<void*, SizeType>> allocations() {
            uint8_t* first = head;

            while (first >= m_storage.begin() && first < m_storage.end()) {
                auto len = *reinterpret_cast<SizeType*>(first);
                co_yield std::pair<void*, int>(first, len);
                first += len;
            }
        }

        // [size of allocation] [...data...] [size of next allocation] [...data...]
        span<uint8_t> m_storage;
        uint8_t* head = nullptr;
        uint8_t* end = m_storage.begin();
    };

    ring_allocator<uint8_t> m_alloc;
};

struct string_event {
    const char* data;
    friend void log(const string_event& ev, detail::any_logger& logger) {
        logger.log("string_event", ev.data);
    }
};

template<TraceEvent T>
void publish(const T& event) {
    log(event, debug::default_log());
}
} // namespace tos::debug