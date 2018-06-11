//
// Created by fatih on 4/26/18.
//

#pragma once

#include <drivers/common/usart.hpp>
#include "src/uart_priv.h"

namespace tos
{
    namespace lx106
    {
        struct uart0
        {
        public:
            static void enable();

            static void disable();
            static void set_baud_rate(usart_baud_rate);

            static void options(usart_parity, usart_stop_bit);

            static int read(char *buf, size_t sz);

            static int write(const char *buf, size_t sz)
            {
            }
        };
    }

    inline lx106::uart0* open_impl(devs::usart_t<0>, usart_baud_rate rate)
    {
        ::UART_init((UartBautRate)rate.rate, UartBautRate::BIT_RATE_300, 0);
        lx106::uart0::set_baud_rate(rate);
        return nullptr;
    }
}

namespace tos
{
    namespace lx106
    {
        void uart0::enable() {
        }

        void uart0::disable() {
        }

        void uart0::set_baud_rate(usart_baud_rate baud) {
            ::UART_SetBaudrate(0, baud.rate);
        }

        void uart0::options(usart_parity parity, usart_stop_bit stop) {
            auto cvt_par = [](usart_parity p)
            {
                switch (p)
                {
                    case usart_parity::disabled: return UartParityMode::NONE_BITS;
                    case usart_parity::reserved: return UartParityMode::NONE_BITS;
                    case usart_parity::even: return UartParityMode::EVEN_BITS;
                    case usart_parity::odd: return UartParityMode::ODD_BITS;
                }
            };
            auto cvt_stop = [](usart_stop_bit s)
            {
                switch (s)
                {
                    case usart_stop_bit::one: return UartStopBitsNum::ONE_STOP_BIT;
                    case usart_stop_bit::two: return UartStopBitsNum::TWO_STOP_BIT;
                }
            };
            ::UART_SetParity(0, cvt_par(parity));
            ::UART_SetStopBits(0, cvt_stop(stop));
        }
    }
}