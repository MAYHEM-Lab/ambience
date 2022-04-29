#include <common/clock.hpp>
#include <cstdarg>
#include <cstdio>
#include <lwip/init.h>
#include <tos/debug/log.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/if_adapter.hpp>

extern "C" {
unsigned char debug_flags = LWIP_DBG_LEVEL_ALL;

void tos_assert(const char* fmt){
    tos::debug::panic(fmt);
}

uint32_t sys_now() {
    using namespace std::chrono;
    auto res = duration_cast<milliseconds>(
                   tos::lwip::global::system_clock->now().time_since_epoch())
                   .count();
    return res;
}
}