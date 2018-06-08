//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once

#include "gpio.hpp"
#include <drivers/common/usart.hpp>

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
            constexpr uart(usart_constraint, gpio::pin_t rx, gpio::pin_t tx) noexcept
            {
            }

        private:

        };
    }
}
