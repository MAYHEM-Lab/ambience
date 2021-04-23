#include <common/clock.hpp>
#include <cstdarg>
#include <cstdio>
#include <lwip/init.h>
#include <tos/debug/log.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/if_adapter.hpp>

extern "C" {
unsigned char debug_flags = LWIP_DBG_LEVEL_ALL;

int tos_log_printf(const char* fmt, ...) {
    static char buf[1024];

    va_list myargs;
    va_start(myargs, fmt);
    auto ret = vsnprintf(buf, std::size(buf), fmt, myargs);
    LOG(buf);
    va_end(myargs);

    return ret;
}

uint32_t sys_now() {
    using namespace std::chrono;
    auto res = duration_cast<milliseconds>(
                   tos::lwip::global::system_clock->now().time_since_epoch())
                   .count();
    return res;
}
}