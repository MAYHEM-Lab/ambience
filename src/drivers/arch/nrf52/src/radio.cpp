//
// Created by Mehmet Fatih BAKIR on 09/06/2018.
//

#include <radio.hpp>
#include <radio_config.h>
#include <stdint.h>
#include <nrf52.h>
#include <mdk/nrf.h>

namespace tos
{
    namespace arm
    {
        radio::radio()
        {
            radio_configure();
        }

        void radio::transmit(uint32_t data)
        {
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);
            // send the packet:
            NRF_RADIO->EVENTS_READY = 0U;
            NRF_RADIO->TASKS_TXEN   = 1;

            while (NRF_RADIO->EVENTS_READY == 0U)
            {
                // wait
            }
            NRF_RADIO->EVENTS_END  = 0U;
            NRF_RADIO->TASKS_START = 1U;

            while (NRF_RADIO->EVENTS_END == 0U)
            {
                // wait
            }

            NRF_RADIO->EVENTS_DISABLED = 0U;
            // Disable radio
            NRF_RADIO->TASKS_DISABLE = 1U;

            while (NRF_RADIO->EVENTS_DISABLED == 0U)
            {
                // wait
            }
        }

        uint32_t radio::receive()
        {
            uint32_t data{};
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);
            uint32_t result = 0;

            NRF_RADIO->EVENTS_READY = 0U;
            // Enable radio and wait for ready
            NRF_RADIO->TASKS_RXEN = 1U;

            while (NRF_RADIO->EVENTS_READY == 0U)
            {
                // wait
            }
            NRF_RADIO->EVENTS_END = 0U;
            // Start listening and wait for address received event
            NRF_RADIO->TASKS_START = 1U;

            // Wait for end of packet or buttons state changed
            while (NRF_RADIO->EVENTS_END == 0U)
            {
                // wait
            }

            if (NRF_RADIO->CRCSTATUS == 1U)
            {
                result = data;
            }
            NRF_RADIO->EVENTS_DISABLED = 0U;
            // Disable radio
            NRF_RADIO->TASKS_DISABLE = 1U;

            while (NRF_RADIO->EVENTS_DISABLED == 0U)
            {
                // wait
            }
            return result;
        }
    }
}