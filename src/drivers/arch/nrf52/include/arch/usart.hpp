//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once

#include "gpio.hpp"
#include <common/usart.hpp>
#include <tos/mutex.hpp>
#include <tos/span.hpp>
#include <common/driver_base.hpp>

namespace tos
{
    namespace nrf52
    {
        using usart_constraint =
            ct_map<usart_key_policy,
                    el_t<usart_baud_rate, const usart_baud_rate&>,
                    el_t<usart_parity, const usart_parity&>,
                    el_t<usart_stop_bit, const usart_stop_bit&>>;

        class uart : public self_pointing<uart>
        {
        public:
            explicit uart(usart_constraint&& config, gpio::pin_type rx = 8_pin, gpio::pin_type tx = 6_pin) noexcept;

            uart(const uart&) = delete;
            uart(uart&&);

            void write(span<const char> buf);
            span<char> read(span<char> buf);

            ~uart();

        private:

            void handle_callback(const void *p_event);

            tos::mutex m_write_busy;
            tos::semaphore m_write_sync;

            tos::mutex m_read_busy;
            tos::semaphore m_read_sync;
        };
    }

    inline nrf52::uart open_impl(devs::usart_t<0>, nrf52::usart_constraint&& c)
    {
        return nrf52::uart{ std::move(c) };
    }
}
