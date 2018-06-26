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

            static int read(char *buf, size_t sz);

            static int write(const char *buf, size_t sz);
        };
    }

    inline lx106::uart0* open_impl(devs::usart_t<0>, usart_baud_rate rate)
    {

        ::uart_init(UartBautRate::BIT_RATE_19200, UartBautRate::BIT_RATE_19200);
        //::UART_init((UartBautRate)rate.rate, UartBautRate::BIT_RATE_300, 0);
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
    }
}