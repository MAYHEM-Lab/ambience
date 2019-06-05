//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once
#include <nrf_gpio.h>
#include <nrf.h>
#include <common/gpio.hpp>
#include <tos/compiler.hpp>

namespace tos
{
    namespace nrf52
    {
        struct pin_t
        {
            NRF_GPIO_Type* m_port;
            uint8_t m_pin;
        };

        namespace detail
        {
            inline int to_sdk_pin(const pin_t& pin)
            {
                if (pin.m_pin == 0xFF) return 0xFF;
                int num = pin.m_pin;
#if defined(NRF_P1)
                if (pin.m_port == NRF_P1) num += 32;
#endif
                return num;
            }
        }

        class gpio
        {
        public:
            using pin_type = pin_t;

            bool read(pin_type pin);

            static void init();

            /**
             * Sets the given pin to be an input
             */
            static void set_pin_mode(pin_type pin, pin_mode::input_t);

            /**
             * Sets the given pin to be an output
             */
            static void set_pin_mode(pin_type pin, pin_mode::output_t);

            /**
             * Sets the given output pin to digital high
             */
            static void write(pin_type pin, digital::high_t);

            /**
             * Sets the given output pin to digital low
             */
            static void write(pin_type pin, digital::low_t);

            static void write(pin_type pin, bool val);
        };
    }

    inline nrf52::gpio* open_impl(devs::gpio_t)
    {
        nrf52::gpio::init();
        return nullptr;
    }

    inline nrf52::pin_t operator""_pin(unsigned long long pin)
    {
        if (pin == 255) return { nullptr, 0xFF };
#if defined(NRF_P1)
        NRF_GPIO_Type* port = (pin >= 32) ? NRF_P1 : NRF_P0;
#else
        NRF_GPIO_Type* port = NRF_P0;
#endif
        return { port, uint8_t(pin % 32) };
    }
}

// IMPL

namespace tos
{
    namespace nrf52
    {
        inline bool gpio::read(pin_type pin) {
            return pin.m_port->IN & (1UL << pin.m_pin);
        }

        inline void gpio::write(pin_type pin, digital::low_t)
        {
            pin.m_port->OUTCLR = (1UL << pin.m_pin);
        }

        inline void gpio::write(pin_type pin, digital::high_t)
        {
            pin.m_port->OUTSET = (1UL << pin.m_pin);
        }

        inline void gpio::write(pin_type pin, bool val)
        {
            if (!val)
            {
                return write(pin, std::false_type{});
            }
            return write(pin, std::true_type{});
        }

        inline void gpio::set_pin_mode(pin_type pin, pin_mode::output_t)
        {
            pin.m_port->PIN_CNF[pin.m_pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        }

        inline void gpio::set_pin_mode(pin_type pin, pin_mode::input_t)
        {
            pin.m_port->PIN_CNF[pin.m_pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                    | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        }

        inline void gpio::init()
        {
            NRF_P0->OUTSET = UINT32_MAX;
#ifdef NRF_P1
            NRF_P1->OUTSET = UINT32_MAX;
#endif
        }
    }
}
