#pragma once

#include "tos/debug/log.hpp"
#include "tos/generator.hpp"
#include <memory>
#include <tos/debug/detail/logger_base.hpp>
#include <type_traits>
#include <tos/append_only_ring.hpp>

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

    Generator<std::pair<const detail::model_vtbl*, const void*>> iter_events() {
        for (auto [ptr, size] : m_alloc.allocations()) {
            auto ev = static_cast<trace_event*>(ptr);
            co_yield std::pair(detail::ev_vtbl_start + ev->m_evt_id, ev->m_data);
        }
    }

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

    append_only_ring<uint8_t> m_alloc;
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