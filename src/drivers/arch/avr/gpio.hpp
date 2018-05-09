//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <stdint.h>
#include <avr/io.h>

namespace tos
{
    struct pin_t
    {
        decltype((PORTD))& port;
        decltype((PORTB))& dir;
        uint8_t pin;
    };

    inline pin_t from_gpio_num(uint16_t gpio) {
        if (gpio < 8) {
            return {PORTD, DDRD, uint8_t(gpio)};
        } else if (gpio < 14) {
            return {PORTB, DDRB, uint8_t(gpio - 8)};
        } else if (gpio < 20) {
            return {PORTC, DDRC, uint8_t(gpio - 14)};
        }
        // TODO: report error
        return {PORTD, DDRD, 0};
    }

    namespace tos_literals
    {
        inline pin_t operator""_pin(unsigned long long pin)
        {
            return from_gpio_num(pin);
        }
    }

    enum class pin_mode_t
    {
        out,
        in
    };

    namespace avr
    {
        class gpio
        {
        public:
            using digital_io_t = bool;
            using pin_type = pin_t;

            void set_pin_mode(pin_t, pin_mode_t);
            void write(pin_t, digital_io_t);
        };
    }
}

///// Implementation

#include <drivers/common/gpio.hpp>

namespace tos
{
    namespace avr
    {
        inline void gpio::set_pin_mode(pin_t pin, pin_mode_t mode)
        {
            if (mode == pin_mode_t::out)
            {
                pin.dir |= (1 << pin.pin);
            }
            else // if (mode == pin_mode_t::in)
            {
                pin.dir &= ~(1 << pin.pin);
            }
        }

        inline void gpio::write(pin_t pin, gpio::digital_io_t what)
        {
            if (what)
            {
                pin.port |= (1 << pin.pin);
            }
            else {
                pin.port &= ~(1 << pin.pin);
            }
        }
    }
}