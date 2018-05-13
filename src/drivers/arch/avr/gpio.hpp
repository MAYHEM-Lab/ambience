//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <tos/function_ref.hpp>
#include <avr/interrupt.h>
#include <common/gpio.hpp>

namespace tos
{
    namespace avr
    {
        struct port
        {
            decltype((PORTD))& data;
            decltype((DDRD))& dir;
            decltype((PIND))& pin;
        };

        struct ports
        {
            inline static port* D()
            {
                static port res { PORTD, DDRD, PIND };
                return &res;
            }
            inline static port* B()
            {
                static port res { PORTB, DDRB, PINB };
                return &res;
            }
            inline static port* C()
            {
                static port res { PORTC, DDRC, PINC };
                return &res;
            }
        };
    }

    struct pin_t
    {
        avr::port* port;
        uint8_t pin;
    };

    inline pin_t from_gpio_num(uint16_t gpio) {
        if (gpio < 8) {
            return { avr::ports::D(), uint8_t(gpio) };
        } else if (gpio < 14) {
            return { avr::ports::B(), uint8_t(gpio - 8)};
        } else if (gpio < 20) {
            return { avr::ports::C(), uint8_t(gpio - 14)};
        }
        // TODO: report error
        return { nullptr, 0 };
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
        in,
        in_pullup
    };

    enum class pin_change
    {
        falling = 2,
        rising = 3,
        any = 1,
        low = 0
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

            digital_io_t read(const pin_t&);

            void attach_interrupt(const pin_t& pin, pin_change p, function_ref<void()> fun);
        };
    }

    inline avr::gpio* open_impl(devs::gpio_t)
    {
        return nullptr;
    }
}

///// Implementation

namespace tos
{
    namespace avr
    {
        inline void gpio::set_pin_mode(pin_t pin, pin_mode_t mode)
        {
            if (mode == pin_mode_t::out)
            {
                pin.port->dir |= (1 << pin.pin);
            }
            else if (mode == pin_mode_t::in)
            {
                pin.port->dir &= ~(1 << pin.pin);
            }
            else // if (mode == pin_mode_t::in_pullup)
            {
                set_pin_mode(pin, pin_mode_t::in);
                write(pin, true);
            }
        }

        inline void gpio::write(pin_t pin, gpio::digital_io_t what)
        {
            if (what)
            {
                pin.port->data |= (1 << pin.pin);
            }
            else {
                pin.port->data &= ~(1 << pin.pin);
            }
        }

        inline gpio::digital_io_t gpio::read(const pin_t& pin)
        {
            uint8_t port = pin.port->pin;
            return port & (1 << pin.pin);
        }

        extern tos::function_ref<void()> exint_handlers[2];
        inline void gpio::attach_interrupt(const pin_t& pin, pin_change p, tos::function_ref<void()> handler)
        {
            if (pin.port != ports::D()) return;
            if (pin.pin != 2 && pin.pin != 3) return;
            auto mask = uint8_t(p);
            mask <<= ((pin.pin-2) * 2);
            exint_handlers[pin.pin - 2] = handler;
            EICRA |= mask;
            EIMSK |= (1 << (pin.pin-2));
        }
    }
}
