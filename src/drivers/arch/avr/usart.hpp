//
// Created by fatih on 4/11/18.
//

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tos/devices.hpp>
#include <tos/char_stream.hpp>
#include <drivers/common/usart.hpp>
#include <drivers/common/tty.hpp>
#include <tos/span.hpp>

namespace tos {

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
            static void set_baud_rate(usart_baud_rate);

            static void options(usart_modes, usart_parity, usart_stop_bit);

            static int read(span<char> buf);

            static int write(span<const char> buf);

        private:

            usart0() = default;
        };

        void write_sync(const char* x, size_t len);
    }

    inline avr::usart0* open_impl(devs::usart_t<0>, usart_baud_rate rate)
    {
        avr::usart0::set_baud_rate(rate);
        return nullptr;
    }

    template <class BaseUsartT>
    class usart final {
    public:
        int read(span<char> buf) { return BaseUsartT::read(buf); }
        int write(span<const char> buf) { return BaseUsartT::write(buf); }
    };

    inline usart<avr::usart0>* open_impl(devs::tty_t<0>)
    {
        return nullptr;
    }
}
