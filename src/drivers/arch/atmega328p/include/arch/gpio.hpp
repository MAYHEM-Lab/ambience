//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <avr/interrupt.h>
#include <avr/io.h>
#include <common/driver_base.hpp>
#include <common/gpio.hpp>
#include <cstdint>
#include <tos/function_ref.hpp>

namespace tos {
namespace avr {
struct port {
    decltype((PORTB))& data;
    decltype((DDRB))& dir;
    decltype((PINB))& pin;
};

struct ports {
    inline static port* B() {
        static port res{PORTB, DDRB, PINB};
        return &res;
    }

#if defined(PORTC)
    inline static port* C() {
        static port res{PORTC, DDRC, PINC};
        return &res;
    }
#endif
#if defined(PORTD)
    inline static port* D() {
        static port res{PORTD, DDRD, PIND};
        return &res;
    }
#endif
};
} // namespace avr

struct pin_t {
    avr::port* port;
    uint8_t pin;
};

constexpr bool operator==(const pin_t& a, const pin_t& b) {
    return a.port == b.port && a.pin == b.pin;
}

constexpr bool operator!=(const pin_t& a, const pin_t& b) {
    return !(a == b);
}

inline pin_t from_gpio_num(uint16_t gpio) {
#if defined(PORTD)
    if (gpio < 8) {
        return {avr::ports::D(), uint8_t(gpio)};
    }
#endif
#if defined(PORTB)
    if (gpio < 14) {
        return {avr::ports::B(), uint8_t(gpio - 8)};
    }
#endif
#if defined(PORTC)
    if (gpio < 20) {
        return {avr::ports::C(), uint8_t(gpio - 14)};
    }
#endif
    // TODO: report error
    return {nullptr, 0};
}

namespace tos_literals {
inline pin_t operator""_pin(unsigned long long pin) {
    return from_gpio_num(pin);
}
} // namespace tos_literals

namespace avr {

enum class pin_change_values
{
    falling = 2,
    rising = 3,
    any = 1,
    low = 0
};

class gpio : public self_pointing<gpio> {
public:
    using digital_io_t = bool;
    using pin_type = pin_t;

    void set_pin_mode(pin_t, pin_mode::input_t);
    void set_pin_mode(pin_t, pin_mode::output_t);
    void set_pin_mode(pin_t, pin_mode::in_pullup_t);

    /**
     * Atmega328p doesn't have pulldown resistors
     */
    void set_pin_mode(pin_t, pin_mode::in_pulldown_t) = delete;

    void write(pin_t, digital_io_t);

    digital_io_t read(const pin_t&);
};
} // namespace avr

inline avr::gpio open_impl(devs::gpio_t) {
    return {};
}
} // namespace tos

///// Implementation

namespace tos {
namespace avr {
inline void gpio::set_pin_mode(pin_t pin, pin_mode::input_t) {
    pin.port->dir &= ~(1 << pin.pin);
}

inline void gpio::set_pin_mode(pin_t pin, pin_mode::output_t) {
    pin.port->dir |= (1 << pin.pin);
}

inline void gpio::set_pin_mode(pin_t pin, pin_mode::in_pullup_t) {
    set_pin_mode(pin, pin_mode::in);
    write(pin, true);
}

inline void gpio::write(pin_t pin, gpio::digital_io_t what) {
    if (what) {
        pin.port->data |= (1 << pin.pin);
    } else {
        pin.port->data &= ~(1 << pin.pin);
    }
}

inline gpio::digital_io_t gpio::read(const pin_t& pin) {
    uint8_t port = pin.port->pin;
    return port & (1 << pin.pin);
}
} // namespace avr
} // namespace tos
