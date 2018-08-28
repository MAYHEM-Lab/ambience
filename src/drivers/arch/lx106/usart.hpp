//
// Created by fatih on 4/26/18.
//

#pragma once

#include <drivers/common/usart.hpp>
#include <tos/span.hpp>
#include "src/new_uart_priv.h"
#include <tos/driver_traits.hpp>
#include <tos/ft.hpp>

namespace tos
{
    namespace esp82
    {
        using usart_constraint =
        ct_map<usart_key_policy,
                el_t<usart_baud_rate, const usart_baud_rate&>,
                el_t<usart_parity, const usart_parity&>,
                el_t<usart_stop_bit, const usart_stop_bit&>>;

        struct uart0
        {
        public:
            uart0(usart_constraint);

            static void enable();

            static void disable();

            static int write(span<const char>);

            uart0*operator->() {return this;}
            uart0&operator*() {return *this;}
        };

        struct sync_uart0
        {
            static int write(span<const char>);
        };

        static_assert(driver_traits<uart0>::has_write{}, "uart must have write!");
        static_assert(!driver_traits<uart0>::has_read{}, "uart must not have read!");
    }

    inline esp82::uart0 open_impl(devs::usart_t<0>, esp82::usart_constraint params)
    {
        return { std::move(params) };
    }
}


namespace tos
{
    namespace esp82
    {
        inline uart0::uart0(usart_constraint params) {
            ::uart0_open(get<usart_baud_rate>(params).rate, UART_FLAGS_8N1);
            //::uart_init(UartBautRate::BIT_RATE_19200, UartBautRate::BIT_RATE_19200);
            //::UART_init((UartBautRate)rate.rate, UartBautRate::BIT_RATE_300, 0);
            //lx106::uart0::set_baud_rate(rate);
        }

        inline void uart0::enable() {
        }

        inline void uart0::disable() {
        }

        inline int ICACHE_FLASH_ATTR uart0::write(tos::span<const char> buf) {
            //::uart0_tx_buffer((uint8 *)buf.data(), buf.size());
            ::uart0_tx_buffer_sync((const uint8_t*)buf.data(), buf.size());
            tos::this_thread::yield();
            return buf.size();
        }

        inline int sync_uart0::write(span<const char> buf) {
            ::uart0_tx_buffer_sync((const uint8_t*)buf.data(), buf.size());
            return buf.size();
        }
    }
}