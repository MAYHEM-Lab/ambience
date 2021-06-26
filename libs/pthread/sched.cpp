#include <sched.h>
#include <tos/ft.hpp>

extern "C" {
void sched_yield() {
    tos::this_thread::yield();
}
}