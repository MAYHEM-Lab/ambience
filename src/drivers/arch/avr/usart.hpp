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
        enum class usart_modes : uint8_t {
            async = 0,
            sync = 0b01,
            reserved = 0b10,
            spi_master = 0b11
        };

        constexpr uint8_t usart_control(usart_modes mode, usart_parity parity, usart_stop_bit stop) {
            // only support 8 bit chars atm
            return ((uint8_t) mode << 6) | ((uint8_t) parity << 4) | ((uint8_t) stop << 3) | (0b11 << 1);
        }

        using usart_constraint =
        ct_map<usart_key_policy,
                el_t<usart_baud_rate, const usart_baud_rate&>,
                el_t<usart_parity, const usart_parity&>,
                el_t<usart_stop_bit, const usart_stop_bit&>>;

        class usart0 {
        public:
            static void enable();

            static void disable();
            static void set_baud_rate(usart_baud_rate);

            static void options(usart_modes, usart_parity, usart_stop_bit);

            static span<char> read(span<char> buf);

            static int write(span<const char> buf);

        private:

            usart0() = default;
        };

        void write_sync(span<const char>);
    }

    inline avr::usart0 open_impl(devs::usart_t<0>, avr::usart_constraint&& rate)
    {
        avr::usart0::set_baud_rate(get<usart_baud_rate>(rate));
        avr::usart0::options(
                tos::avr::usart_modes::async,
                get<usart_parity>(rate),
                get<usart_stop_bit>(rate));
        avr::usart0::enable();
        return {};
    }
}
