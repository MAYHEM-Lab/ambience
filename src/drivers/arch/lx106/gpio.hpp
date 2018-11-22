//
// Created by fatih on 7/1/18.
//

#pragma once

#include <common/gpio.hpp>
#include <tos/compiler.hpp>

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
            using pin_type = pin_t;

            gpio();

            static void set_pin_mode(pin_t, pin_mode::input_t);
            static void set_pin_mode(pin_t, pin_mode::output_t);
            static void set_pin_mode(pin_t, pin_mode::in_pullup_t);
            static void set_pin_mode(pin_t, pin_mode::in_pulldown_t) = delete;

            static bool read(const pin_t& pin);

            /**
             * Sets the given output pin to digital high
             */
            static void write(pin_t pin, digital::high_t);

            /**
             * Sets the given output pin to digital low
             */
            static void write(pin_t pin, digital::low_t);

            static void write(pin_t pin, bool val);
        };
    }

    inline tos::esp82::gpio open_impl(tos::devs::gpio_t)
    {
        return {};
    }
}

namespace tos
{
    namespace esp82
    {
        inline gpio::gpio() {
            gpio_init();
        }

        inline uintptr_t convert(pin_t pin)
        {
            static uint8_t esp8266_gpioToFn[16] =
                    {0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10};

            return PERIPHS_IO_MUX + esp8266_gpioToFn[pin.pin];
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::input_t) {
            PIN_FUNC_SELECT(convert(pin), 0);
            gpio_output_set(0, 0, 0, 1 << pin.pin);
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::in_pullup_t) {
            set_pin_mode(pin, pin_mode::in);
            PIN_PULLUP_EN(convert(pin));
        }

        inline void gpio::set_pin_mode(pin_t pin, tos::pin_mode::output_t) {
            gpio_output_set(0, 0, 1 << pin.pin, 0);
        }

        bool ALWAYS_INLINE gpio::read(const pin_t &pin) {
            return (gpio_input_get() >> pin.pin) & 1;
        }

        void ALWAYS_INLINE gpio::write(pin_t pin, digital::high_t) {
            gpio_output_set(1 << pin.pin, 0, 1 << pin.pin, 0);
        }

        void ALWAYS_INLINE gpio::write(pin_t pin, digital::low_t) {
            gpio_output_set(0, 1 << pin.pin, 1 << pin.pin, 0);
        }

        inline void gpio::write(pin_t pin, bool val) {
            if (!val)
            {
                return write(pin, std::false_type{});
            }
            return write(pin, std::true_type{});
        }
    }
}