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

            void enable_receive();
            void enable_transmit();
            void disable_radio();

            void disable_receive();

            void transmit(uint32_t data);

            uint32_t receive();

        private:

            void enable_interrupts();
            void disable_interrupts();

        };
    }
}
