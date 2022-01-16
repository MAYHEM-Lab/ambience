#include "common.hpp"
#include "tos/arm/assembly.hpp"
#include "tos/debug/log.hpp"
#include "tos/interrupt.hpp"
#include <chrono>
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
        // unlink();
    }

    ble_npl_event_fn* fn;
    void* arg;
    int outstanding = 0;
    bool m_is_linked = false;

    void link() {
        if (is_linked()) {
            LOG_ERROR(this, "Already linked!!!!");
            while (true) {
                arm::nop();
            }
        }
        m_is_linked = true;
    }

    void unlink() {
        if (!is_linked()) {
            LOG_ERROR(this, "Already unlinked!!!!");
            while (true) {
                arm::nop();
            }
        }
        m_is_linked = false;
    }

    bool is_linked() const {
        return m_is_linked;
    }

    void run() {
        fn(self());
    }

    ble_npl_event* self() {
        return reinterpret_cast<ble_npl_event*>(this);
    }
};

struct event_queue {
    void push_event(event& ev) {
        tos::int_guard ig;

        LOG_TRACE("Push", &ev, ev.outstanding);
        if (ev.is_linked()) {
            LOG_TRACE("Already linked");
            return;
        }

        ev.outstanding++;
        if (ev.outstanding > 1) {
            LOG_ERROR("Pushed multiple", &ev);
            while (true) {
                arm::nop();
            }
        }

        events.push_back(ev);
        ev.link();
        sem.up(ig);

        LOG_TRACE(&ev, "Push Success", ev.outstanding, events.size(), ev.is_linked(), ev.prev, ev.next);
    }

    bool remove_event(event& ev) {
        LOG_TRACE("Rm", &ev, ev.outstanding);

        tos::int_guard ig;

        if (!ev.is_linked() && ev.outstanding != 0) {
            LOG_ERROR(&ev, "Not linked, but non-zero outstanding");
            while (true) {
                arm::nop();
            }
        }

        if (!ev.is_linked()) {
            LOG_TRACE(&ev, "Not linked");
            return false;
        }

        auto down_res = try_down_isr(sem);

        if (!down_res) {
            LOG_ERROR(&ev, "Cannot get sem, ERROR");
            while (true) {
                arm::nop();
            }

            return false;
        }

        auto it = events.unsafe_find(ev);
        events.erase(it);

        ev.unlink();
        ev.outstanding--;
        if (ev.outstanding != 0) {
            LOG_ERROR("Removed multiple!");
            while (true) {
                arm::nop();
            }
        }

        LOG_TRACE(&ev, "Remove success");

        return true;
    }

    bool empty() const {
        return events.empty();
    }

    event* pop() {
        tos::int_guard ig;
        sem.down(ig);
        auto& res = events.front();
        events.pop_front();
        res.unlink();
        LOG_TRACE("Rm", &res);
        res.outstanding--;
        if (res.outstanding != 0) {
            LOG_ERROR("Removed multiple!");
            while (true) {
                arm::nop();
            }
        }
        return &res;
    }

    template<class... Args>
    event* pop(Args&&... args) {
        tos::int_guard ig;
        auto down_res = sem.down(std::forward<Args>(args)..., ig);
        if (down_res != sem_ret::normal) {
            return nullptr;
        }
        auto& res = events.front();
        events.pop_front();
        res.unlink();
        res.outstanding--;
        LOG_TRACE("Rm", &res, res.outstanding);
        if (res.outstanding != 0) {
            LOG_ERROR("Removed multiple!");
            while (true) {
                arm::nop();
            }
        }
        return &res;
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
    static_assert(sizeof(tos::nimble::event_queue) <= BLE_NPL_EVENTQ_SIZE);
    new (&evq->buffer) tos::nimble::event_queue;
    evq->initd = 0x12345678;
}

struct ble_npl_event* ble_npl_eventq_get(struct ble_npl_eventq* evq, ble_npl_time_t tmo) {
    if (tmo == BLE_NPL_TIME_FOREVER) {
        return get_event_queue(evq).pop()->self();
    } else if (tmo == 0) {
        LOG_ERROR("Dunno");
    } else {
        auto el =
            get_event_queue(evq).pop(*tos::nimble::alarm, std::chrono::milliseconds(tmo));
        if (el) {
            return el->self();
        }
    }
    return nullptr;
}

void ble_npl_eventq_put(struct ble_npl_eventq* evq, struct ble_npl_event* ev) {
    get_event_queue(evq).push_event(get_event(ev));
}

void ble_npl_eventq_remove(struct ble_npl_eventq* evq, struct ble_npl_event* ev) {
    tos::int_guard ig;
    get_event_queue(evq).remove_event(get_event(ev));
}

void ble_npl_event_init(struct ble_npl_event* ev, ble_npl_event_fn* fn, void* arg) {
    static_assert(sizeof(tos::nimble::event) <= BLE_NPL_EVENT_SIZE);
    auto ptr = new (&ev->buffer) tos::nimble::event(fn, arg);
    ev->initd = 0x12345678;
    Assert(ptr->self() == ev);
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