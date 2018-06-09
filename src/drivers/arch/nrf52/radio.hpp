//
// Created by Mehmet Fatih BAKIR on 09/06/2018.
//

#pragma once

#include <stdint.h>

namespace tos
{
    namespace arm
    {
        class radio
        {
        public:
            radio();

            void transmit(uint32_t data);

            uint32_t receive();
        };
    }
}
