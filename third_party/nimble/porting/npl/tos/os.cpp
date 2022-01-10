#include "tos/thread.hpp"
#include "tos/threading_state.hpp"
#include <nimble/nimble_npl.h>
#include <tos/ft.hpp>

extern "C" {
bool ble_npl_os_started(void) {
    return true;
}

void ble_npl_task_yield(void) {
    tos::this_thread::yield();
}

void* ble_npl_get_current_task_id(void) {
    return tos::self();
}

void do_assert(int rc) {
    if (!rc) {
        asm volatile ("bkpt 0");
        Assert(false);
    }
}
}