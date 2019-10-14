//
// Created by fatih on 4/16/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/timer.hpp>
#include <cstdint>
#include <tos/function_ref.hpp>

namespace tos {
namespace avr {
class timer1
    : public self_pointing<timer1>
    , public non_copy_movable {
public:
    void set_frequency(uint16_t hertz);

    void enable();

    void disable();

    void set_callback(const function_ref<void()>&);

    uint16_t get_counter() const;
    uint16_t get_period() const;

    ~timer1();
private:
};
} // namespace avr

inline avr::timer1 open_impl(devs::timer_t<1>) {
    return {};
}
} // namespace tos