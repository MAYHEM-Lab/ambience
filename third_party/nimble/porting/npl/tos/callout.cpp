#include "tos/debug/log.hpp"
#include "tos/function_ref.hpp"
#include "tos/interrupt.hpp"
#include <common/alarm.hpp>
#include <new>
#include <nimble/nimble_npl.h>

namespace tos::nimble {
tos::any_alarm* alarm;
struct timer_info {
    std::optional<sleeper> s;

    ble_npl_callout* self() {
        return reinterpret_cast<ble_npl_callout*>(this);
    }

    void cb() {
        s.reset();
        if (self()->evq) {
            ble_npl_eventq_put(self()->evq, &self()->ev);
        } else {
            ble_npl_event_run(&self()->ev);
        }
    }

    void reset(int ticks, const int_guard& ig) {
        stop(ig);

        s.emplace(ticks, tos::mem_function_ref<&tos::nimble::timer_info::cb>(*this));
        self()->ticks = ble_npl_time_get() + ticks;

        tos::nimble::alarm->set_alarm(*s);
    }

    void stop(const int_guard& ig) {
        if (!s) {
            return;
        }

        tos::nimble::alarm->cancel(tos::nimble::alarm->unsafe_find_handle(*s));
        s.reset();
    }
};

static_assert(sizeof(timer_info) <= BLE_NPL_CALLOUT_SIZE);
static_assert(alignof(timer_info) <= alignof(void*));
} // namespace tos::nimble

tos::nimble::timer_info& get_timer_info(ble_npl_callout* co) {
    return *std::launder(reinterpret_cast<tos::nimble::timer_info*>(co->buffer));
}

extern "C" {
void ble_npl_callout_init(struct ble_npl_callout* co,
                          struct ble_npl_eventq* evq,
                          ble_npl_event_fn* ev_cb,
                          void* ev_arg) {
    ble_npl_event_init(&co->ev, ev_cb, ev_arg);
    auto ptr = new (co->buffer) tos::nimble::timer_info{};
    Assert(ptr->self() == co);
    co->evq = evq;
    co->initd = 0x12345678;
}

ble_npl_error_t ble_npl_callout_reset(struct ble_npl_callout* co, ble_npl_time_t ticks) {
    tos::int_guard ig;
    get_timer_info(co).reset(ticks == 0 ? 1 : ticks, ig);
    return BLE_NPL_OK;
}

void ble_npl_callout_stop(struct ble_npl_callout* co) {
    tos::int_guard ig;
    get_timer_info(co).stop(ig);
}

bool ble_npl_callout_is_active(struct ble_npl_callout* co) {
    return get_timer_info(co).s.has_value();
}

ble_npl_time_t ble_npl_callout_get_ticks(struct ble_npl_callout* co) {
    return co->ticks;
}

ble_npl_time_t ble_npl_callout_remaining_ticks(struct ble_npl_callout* co,
                                               ble_npl_time_t time);

void ble_npl_callout_set_arg(struct ble_npl_callout* co, void* arg);
}