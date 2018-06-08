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
        class uart
        {
        public:
            constexpr uart(usart_config_t<0x7> c, gpio::pin_t rx, gpio::pin_t tx) noexcept
            {
                if (c.m_sb == usart_stop_bit::two)
                {
                }
            }

        private:

        };
    }
}
