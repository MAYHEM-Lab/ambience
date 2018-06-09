//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once

#include "gpio.hpp"
#include <drivers/common/usart.hpp>
#include <tos/mutex.hpp>
#include <tos/span.hpp>

namespace tos
{
    namespace arm
    {
        using usart_constraint =
                ct_map<usart_key_policy,
                        el_t<usart_baud_rate, const usart_baud_rate&>,
                        el_t<usart_parity, const usart_parity&>,
                        el_t<usart_stop_bit, const usart_stop_bit&>>;
        class uart
        {
        public:
            explicit uart(usart_constraint&& config, gpio::pin_t rx = 8, gpio::pin_t tx = 6) noexcept;

            uart(const uart&) = delete;
            uart(uart&&);

            void write(span<const char> buf);
            void read(span<char> buf);

        private:

            void handle_callback(const void *p_event);

            tos::mutex m_write_busy;
            tos::semaphore m_write_sync;

            tos::mutex m_read_busy;
            tos::semaphore m_read_sync;
        };
    }

    inline arm::uart open_impl(devs::usart_t<0>, arm::usart_constraint&& c)
    {
        return arm::uart{ tos::move(c) };
    }
}
