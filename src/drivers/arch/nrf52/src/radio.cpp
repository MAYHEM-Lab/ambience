//
// Created by Mehmet Fatih BAKIR on 09/06/2018.
//

#include <radio.hpp>
#include <radio_config.h>
#include <stdint.h>
#include <nrf52.h>
#include <mdk/nrf.h>
#include <tos/semaphore.hpp>

namespace tos
{
    namespace arm
    {
        struct radio_data
        {
            tos::semaphore ready{0};
            tos::semaphore disabled{0};
            tos::semaphore end{0};
        };

        static radio_data data;

        radio::radio()
        {
            radio_configure();

            NRF_RADIO->INTENSET =
                    RADIO_INTENSET_READY_Msk
                    | RADIO_INTENCLR_END_Msk
                    | RADIO_INTENCLR_DISABLED_Msk;

            this->enable_interrupts();
        }

        void radio::transmit(uint32_t data)
        {
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);

            NRF_RADIO->EVENTS_READY = 0U;
            NRF_RADIO->TASKS_TXEN   = 1;

            tos::arm::data.ready.down();

            NRF_RADIO->EVENTS_END  = 0U;
            NRF_RADIO->TASKS_START = 1U;

            tos::arm::data.end.down();

            NRF_RADIO->EVENTS_DISABLED = 0U;
            NRF_RADIO->TASKS_DISABLE = 1U;

            tos::arm::data.disabled.down();
        }

        uint32_t radio::receive()
        {
            volatile uint32_t data{};
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);
            uint32_t result = 0;

            NRF_RADIO->EVENTS_READY = 0U;
            NRF_RADIO->TASKS_RXEN = 1U;

            tos::arm::data.ready.down();

            NRF_RADIO->EVENTS_END = 0U;
            NRF_RADIO->TASKS_START = 1U;

            tos::arm::data.end.down();

            if (NRF_RADIO->CRCSTATUS == 1U)
            {
                result = data;
            }

            disable_radio();

            return result;
        }

        void radio::disable_radio() {
            NRF_RADIO->EVENTS_DISABLED = 0U;
            NRF_RADIO->TASKS_DISABLE = 1U;

            tos::arm::data.disabled.down();
        }

        void radio::enable_interrupts() {
            NVIC_SetPriority(RADIO_IRQn, 1);
            NVIC_ClearPendingIRQ(RADIO_IRQn);
            NVIC_EnableIRQ(RADIO_IRQn);
        }

        void radio::disable_interrupts() {
            NVIC_DisableIRQ(RADIO_IRQn);
        }
    }
}

extern "C"
{
    void RADIO_IRQHandler(void)
    {
        if (NRF_RADIO->EVENTS_READY && (NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk))
        {
            tos::arm::data.ready.up_isr();
            NRF_RADIO->EVENTS_READY = 0;
        }
        else if (NRF_RADIO->EVENTS_DISABLED && (NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk))
        {
            tos::arm::data.disabled.up_isr();
            NRF_RADIO->EVENTS_DISABLED = 0;
        }
        else if (NRF_RADIO->EVENTS_END && (NRF_RADIO->INTENSET & RADIO_INTENCLR_END_Msk))
        {
            tos::arm::data.end.up_isr();
            NRF_RADIO->EVENTS_END = 0;
        }
    }
}