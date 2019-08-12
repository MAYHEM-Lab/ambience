//
// Created by fatih on 6/26/18.
//

#pragma once

#include "../../../../../../../../../opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/8.3.0/include/stdint.h"

#include <common/timer.hpp>
#include <tos/function_ref.hpp>

extern "C" {
#include <osapi.h>
}

namespace tos {
namespace esp82 {
class timer {
public:
    timer();

    void set_frequency(uint16_t hertz);

    void enable();

    void disable();

    void set_callback(const function_ref<void()>& cb);

    timer* operator->() { return this; }
    timer& operator*() { return *this; }

private:
    function_ref<void()> m_cb;
    uint16_t m_freq;

    os_timer_t m_timer;
};
} // namespace esp82

inline esp82::timer open_impl(devs::timer_t<0>) { return esp82::timer{}; }
} // namespace tos
