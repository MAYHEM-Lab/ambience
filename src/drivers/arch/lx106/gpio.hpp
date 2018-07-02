//
// Created by fatih on 7/1/18.
//

#pragma once

#include <common/gpio.hpp>

extern "C"
{
#include <user_interface.h>
}

namespace tos
{
    namespace esp82
    {
        struct pin_t
        {
            uint8_t pin;
        };

        class gpio
        {
        public:
            gpio();

            static void set_pin_mode(pin_t, pin_mode::input_t);
            static void set_pin_mode(pin_t, pin_mode::output_t);
            static void set_pin_mode(pin_t, pin_mode::in_pullup_t);
            static void set_pin_mode(pin_t, pin_mode::in_pulldown_t) = delete;

            static bool read(const pin_t& pin);
        };
    }
}

namespace tos
{
    namespace esp82
    {
        constexpr const uintptr_t conversion[] = {
            PERIPHS_IO_MUX_GPIO0_U
        };

        inline gpio::gpio() {
            gpio_init();
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::input_t) {
            PIN_FUNC_SELECT(conversion[pin.pin], 0);
            gpio_output_set(0, 0, 0, 1 << pin.pin);
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::in_pullup_t) {
            set_pin_mode(pin, pin_mode::in);
            PIN_PULLUP_EN(GPIO_PIN_ADDR(pin.pin));
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::output_t) {
            gpio_output_set(0, 0, 1 << pin.pin, 0);
        }

        inline bool gpio::read(const pin_t &pin) {
            return (gpio_input_get() >> pin.pin) & 1;
        }
    }
}