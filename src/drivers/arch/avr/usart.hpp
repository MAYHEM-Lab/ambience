//
// Created by fatih on 4/11/18.
//

#pragma once

#include <stdint.h>
#include <stddef.h>

namespace tos
{
    enum class usart_modes : uint8_t
    {
        async = 0,
        sync = 0b01,
        reserved = 0b10,
        spi_master = 0b11
    };

    enum class usart_parity : uint8_t
    {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    enum class usart_stop_bit : uint8_t
    {
        one = 0b0,
        two = 0b1
    };

    constexpr uint8_t usart_control(usart_modes mode, usart_parity parity, usart_stop_bit stop)
    {
        // only support 8 bit chars atm
        return ((uint8_t)mode << 6) | ((uint8_t)parity << 4) | ((uint8_t)stop << 3) | (0b11 << 1);
    }

/**
 * This class manages the AVR USART0 device
 */
    class avr_usart0
    {
    public:
        static void set_baud_rate(uint16_t);
        static void set_2x_rate();
        static void enable();
        static void disable();

        static void set_control(usart_modes, usart_parity, usart_stop_bit);

    private:
        avr_usart0() = default;
    };
}
size_t read_usart(char* buf, size_t len);
