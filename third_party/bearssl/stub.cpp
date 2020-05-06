#include <time.h>
#include <stdint.h>

extern "C" {
[[gnu::weak]]
void tos_this_thread_yield() {}

[[gnu::weak]]
time_t time(time_t*) {
    return {};
}

[[gnu::weak]]
uint32_t tos_rand_source(void) {
    return 4;
}
}