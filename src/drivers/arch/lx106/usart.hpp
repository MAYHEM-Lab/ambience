//
// Created by fatih on 4/26/18.
//

#pragma once

#include <drivers/common/usart.hpp>
#include <tos/span.hpp>
#include "src/new_uart_priv.h"

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

            static int write(span<const char>);
        };
    }

    inline lx106::uart0* open_impl(devs::usart_t<0>, usart_baud_rate rate)
    {
        ::uart0_open(rate.rate, UART_FLAGS_8N1);
        //::uart_init(UartBautRate::BIT_RATE_19200, UartBautRate::BIT_RATE_19200);
        //::UART_init((UartBautRate)rate.rate, UartBautRate::BIT_RATE_300, 0);
        //lx106::uart0::set_baud_rate(rate);
        return nullptr;
    }
}


namespace tos
{
    namespace lx106
    {
        inline void uart0::enable() {
        }

        inline void uart0::disable() {
        }

        inline void ICACHE_FLASH_ATTR uart0::set_baud_rate(usart_baud_rate baud) {
            //::UART_SetBaudrate(0, baud.rate);
        }

        inline int ICACHE_FLASH_ATTR uart0::write(tos::span<const char> buf) {
            ::uart0_tx_buffer((uint8 *)buf.data(), buf.size());
            return buf.size();
        }
    }
}