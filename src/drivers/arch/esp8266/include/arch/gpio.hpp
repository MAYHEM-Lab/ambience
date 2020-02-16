//
// Created by fatih on 7/1/18.
//

#pragma once

#include <common/gpio.hpp>
#include <tos/compiler.hpp>
#include <tos/self_pointing.hpp>

extern "C" {
#include <user_interface.h>
}

namespace tos {
namespace esp82 {
struct pin_t {
    pin_t() = default;
    template <size_t PinId>
    pin_t(tos::gpio_pin<0, PinId>) : pin(PinId), mask(1 << pin) {}
    explicit pin_t(size_t pin_id) : pin(pin_id), mask(1 << pin) {}

    uint8_t pin;
    uint32_t mask;
};
class gpio : public self_pointing<gpio> {
public:
    using pin_type = pin_t;

    static constexpr gpio_port<0, 17> port{};

    gpio();

    void set_pin_mode(pin_t, pin_mode::input_t);
    void set_pin_mode(pin_t, pin_mode::output_t);
    void set_pin_mode(pin_t, pin_mode::in_pullup_t);
    void set_pin_mode(pin_t, pin_mode::in_pulldown_t) = delete;

    bool read(const pin_t& pin);

    /**
     * Sets the given output pin to digital high
     */
    void write(pin_t pin, digital::high_t);

    /**
     * Sets the given output pin to digital low
     */
    void write(pin_t pin, digital::low_t);

    void write(pin_t pin, bool val);
};
} // namespace esp82

inline tos::esp82::gpio open_impl(tos::devs::gpio_t) { return {}; }
} // namespace tos

namespace tos {
namespace esp82 {
inline gpio::gpio() { gpio_init(); }

inline uintptr_t convert(pin_t pin) {
    static uint8_t esp8266_gpioToFn[16] = {0x34,
                                           0x18,
                                           0x38,
                                           0x14,
                                           0x3C,
                                           0x40,
                                           0x1C,
                                           0x20,
                                           0x24,
                                           0x28,
                                           0x2C,
                                           0x30,
                                           0x04,
                                           0x08,
                                           0x0C,
                                           0x10};

    return PERIPHS_IO_MUX + esp8266_gpioToFn[pin.pin];
}

inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::input_t) {
    PIN_FUNC_SELECT(convert(pin), 0);
    gpio_output_set(0, 0, 0, pin.mask);
}

inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::in_pullup_t) {
    set_pin_mode(pin, pin_mode::in);
    PIN_PULLUP_EN(convert(pin));
}

inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::output_t) {
    gpio_output_set(0, 0, pin.mask, 0);
}

bool ALWAYS_INLINE gpio::read(const pin_t& pin) {
    return (GPIO_REG_READ(GPIO_IN_ADDRESS) >> pin.pin) & 1;
}

void ALWAYS_INLINE gpio::write(pin_t pin, digital::high_t) {
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pin.mask);
}

void ALWAYS_INLINE gpio::write(pin_t pin, digital::low_t) {
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pin.mask);
}

inline void gpio::write(pin_t pin, bool val) {
    if (!val) {
        return write(pin, std::false_type{});
    }
    return write(pin, std::true_type{});
}
} // namespace esp82
namespace tos_literals {
    inline esp82::pin_t operator""_pin(unsigned long long pin) {
        return esp82::pin_t{static_cast<uint8_t>(pin)};
    }
} // namespace tos_literals
} // namespace tos