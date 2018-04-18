//
// Created by fatih on 4/11/18.
//

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tos/devices.hpp>
#include <tos/char_stream.hpp>

namespace tos {
    enum class usart_modes : uint8_t {
        async = 0,
        sync = 0b01,
        reserved = 0b10,
        spi_master = 0b11
    };

    enum class usart_parity : uint8_t {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    enum class usart_stop_bit : uint8_t {
        one = 0b0,
        two = 0b1
    };

/**
 * This class manages the AVR USART0 device
 */
    namespace avr {
        constexpr uint8_t usart_control(usart_modes mode, usart_parity parity, usart_stop_bit stop) {
            // only support 8 bit chars atm
            return ((uint8_t) mode << 6) | ((uint8_t) parity << 4) | ((uint8_t) stop << 3) | (0b11 << 1);
        }

        class usart0 {
        public:
            static void enable();

            static void disable();

            static void set_baud_rate(uint32_t);

            static void set_control(usart_modes, usart_parity, usart_stop_bit);

        private:
            usart0() = default;
        };
    }

    avr::usart0* open_impl(devs::usart_t<0>)
    {
        return nullptr;
    }

    class usart final {
    public:
        int read(char *buf, size_t sz);

        char getc();

        int write(const char *buf, size_t sz);

        void putc(char c);
    };
}
