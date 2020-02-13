//
// Created by fatih on 11/6/19.
//

#pragma once

#include <common/gpio.hpp>
#include <ti/drivers/GPIO.h>
#include <tos/self_pointing.hpp>

namespace tos::cc32xx {
struct pin_t {
    uint_fast8_t gpio_num;
};
class gpio : public self_pointing<gpio> {
public:
    using pin_type = pin_t;
    gpio() {
        [[maybe_unused]] static auto _ = [] {
            GPIO_init();
            return 0;
        }();
    }

    bool read(pin_type pin);

    /**
     * Sets the given pin to be an input
     */
    void set_pin_mode(pin_type pin, pin_mode::input_t);

    /**
     * Sets the given pin to be an input
     */
    void set_pin_mode(pin_type pin, pin_mode::in_pulldown_t);

    /**
     * Sets the given pin to be an output
     */
    void set_pin_mode(pin_type pin, pin_mode::output_t);

    /**
     * Sets the given output pin to digital high
     */
    void write(pin_type pin, digital::high_t);

    /**
     * Sets the given output pin to digital low
     */
    void write(pin_type pin, digital::low_t);

    void write(pin_type pin, bool b) {
        if (b) {
            write(pin, digital::high);
        } else {
            write(pin, digital::low);
        }
    }
private:
};
} // namespace tos::cc32xx

// impl

namespace tos::cc32xx {
inline bool gpio::read(pin_type pin) {
    return GPIO_read(pin.gpio_num);
}

inline void gpio::set_pin_mode(pin_type pin, tos::pin_mode::output_t) {
    /* Configure the LED pin */
    GPIO_setConfig(pin.gpio_num, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
}

inline void gpio::set_pin_mode(pin_type pin, pin_mode::input_t) {
    GPIO_setConfig(pin.gpio_num, GPIO_CFG_IN_NOPULL);
}

inline void gpio::set_pin_mode(pin_type pin, pin_mode::in_pulldown_t) {
    GPIO_setConfig(pin.gpio_num, GPIO_CFG_IN_PD);
}

inline void gpio::write(pin_type pin, digital::high_t) {
    GPIO_write(pin.gpio_num, 1);
}

inline void gpio::write(pin_type pin, digital::low_t) {
    GPIO_write(pin.gpio_num, 0);
}
} // namespace tos::cc32xx

namespace tos {
namespace tos_literals {
inline cc32xx::pin_t operator""_pin(unsigned long long pin) {
    return {static_cast<uint_fast8_t>(pin)};
}
} // namespace tos_literals
} // namespace tos