//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once
#include <nrf_gpio.h>
#include <nrf52.h>
#include <drivers/common/gpio.hpp>
#include <tos/compiler.hpp>

namespace tos
{
    namespace arm
    {
        class gpio
        {
        public:
            using pin_t = int;

            static void init();

            /**
             * Sets the given pin to be an input
             */
            static void set_pin_mode(pin_t pin, pin_mode::input_t);

            /**
             * Sets the given pin to be an output
             */
            static void set_pin_mode(pin_t pin, pin_mode::output_t);

            /**
             * Sets the given output pin to digital high
             */
            static void write(pin_t pin, digital::high_t) ALWAYS_INLINE;

            /**
             * Sets the given output pin to digital low
             */
            static void write(pin_t pin, digital::low_t) ALWAYS_INLINE;

            static void write(pin_t pin, bool val);
        };
    }

    arm::gpio* open_impl(devs::gpio_t)
    {
        arm::gpio::init();
        return nullptr;
    }
}

// IMPL

namespace tos
{
    namespace arm
    {
        inline void tos::arm::gpio::write(int pin, digital::low_t)
        {
            NRF_P0->OUTCLR = (1UL << pin);
        }

        inline void gpio::write(int pin, digital::high_t)
        {
            NRF_P0->OUTSET = (1UL << pin);
        }

        inline void gpio::write(int pin, bool val)
        {
            if (!val)
            {
                return write(pin, tos::false_type{});
            }
            return write(pin, tos::true_type{});
        }

        inline void gpio::set_pin_mode(int pin, pin_mode::output_t)
        {
            NRF_P0->PIN_CNF[pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        }

        inline void gpio::set_pin_mode(int pin, pin_mode::input_t)
        {
            NRF_P0->PIN_CNF[pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        }

        inline void gpio::init()
        {
            NRF_P0->OUTSET = UINT32_MAX;
        }
    }
}
