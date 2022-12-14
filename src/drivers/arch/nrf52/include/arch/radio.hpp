//
// Created by Mehmet Fatih BAKIR on 09/06/2018.
//

#pragma once

#include <stdint.h>

namespace tos
{
    namespace nrf52
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

            void set_packet_ptr(volatile void*);
            void set_packet_ptr(void*);

            void stop();

            void enable_interrupts();
            void disable_interrupts();

        };
    }
}
