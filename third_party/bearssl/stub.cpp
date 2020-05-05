#include <time.h>
extern "C" {
[[gnu::weak]]
void tos_this_thread_yield() {}

[[gnu::weak]]
time_t time(time_t*) {
    return {};
}
}