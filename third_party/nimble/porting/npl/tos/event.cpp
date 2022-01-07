#include "tos/debug/log.hpp"
#include <new>
#include <nimble/nimble_npl.h>
#include <tos/intrusive_list.hpp>
#include <tos/semaphore.hpp>

namespace tos::nimble {
namespace {
struct event : list_node<event> {
    event(ble_npl_event_fn* fn, void* arg)
        : fn(fn)
        , arg(arg) {
        unlink();
    }

    ble_npl_event_fn* fn;
    void* arg;

    void run() {
        fn(self());
    }

    ble_npl_event* self() {
        return reinterpret_cast<ble_npl_event*>(this);
    }
};

struct event_queue {
    void push_event(event& ev) {
        events.push_back(ev);
        sem.up();
    }

    void remove_event(event& ev) {
        auto down_res = try_down_isr(sem);

        if (!down_res) {
            LOG_ERROR("");
            return;
        }

        auto it = events.unsafe_find(ev);
        events.erase(it);

        ev.unlink();
    }

    bool empty() const {
        return events.empty();
    }

    event& pop() {
        sem.down();
        auto& res = events.front();
        events.pop_front();
        return res;
    }

private:
    intrusive_list<event> events;
    semaphore sem{0};
};
} // namespace
} // namespace tos::nimble

tos::nimble::event& get_event(ble_npl_event* ev) {
    return *std::launder(reinterpret_cast<tos::nimble::event*>(ev->buffer));
}

tos::nimble::event_queue& get_event_queue(ble_npl_eventq* ev) {
    return *std::launder(reinterpret_cast<tos::nimble::event_queue*>(ev->buffer));
}

extern "C" {
void ble_npl_eventq_init(struct ble_npl_eventq* evq) {
    new (&evq->buffer) tos::nimble::event_queue;
}

struct ble_npl_event* ble_npl_eventq_get(struct ble_npl_eventq* evq, ble_npl_time_t tmo) {
    return get_event_queue(evq).pop().self();
}

void ble_npl_eventq_put(struct ble_npl_eventq* evq, struct ble_npl_event* ev) {
    get_event_queue(evq).push_event(get_event(ev));
}

void ble_npl_eventq_remove(struct ble_npl_eventq* evq, struct ble_npl_event* ev) {
    get_event_queue(evq).remove_event(get_event(ev));
}

void ble_npl_event_init(struct ble_npl_event* ev, ble_npl_event_fn* fn, void* arg) {
    new (&ev->buffer) tos::nimble::event(fn, arg);
}

bool ble_npl_event_is_queued(struct ble_npl_event* ev) {
    return get_event(ev).is_linked();
}

void* ble_npl_event_get_arg(struct ble_npl_event* ev) {
    return get_event(ev).arg;
}

void ble_npl_event_set_arg(struct ble_npl_event* ev, void* arg) {
    get_event(ev).arg = arg;
}

bool ble_npl_eventq_is_empty(struct ble_npl_eventq* evq) {
    return get_event_queue(evq).empty();
}

void ble_npl_event_run(struct ble_npl_event* ev) {
    get_event(ev).run();
}
}