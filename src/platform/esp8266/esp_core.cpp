//
// Created by fatih on 1/9/19.
//

#include <tos/platform.hpp>
extern "C" {
#include <user_interface.h>
}

namespace tos::platform {
[[noreturn]] void force_reset() {
    // esp sdk should reset
    while (true) {
    }
}
} // namespace tos::platform

namespace tos::esp82 {
uint32_t get_clock_speed() {
    return system_get_cpu_freq() * 1'000'000; // megahertz
}
} // namespace tos::esp82