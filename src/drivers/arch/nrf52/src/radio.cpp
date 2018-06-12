//
// Created by Mehmet Fatih BAKIR on 09/06/2018.
//

#include <radio.hpp>
#include <stdint.h>
#include <nrf52.h>
#include <mdk/nrf.h>
#include <tos/semaphore.hpp>

#define PACKET_BASE_ADDRESS_LENGTH  (4UL)                   //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (4UL)                   //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (PACKET_STATIC_LENGTH)  //!< Packet payload maximum size in bytes

#define PACKET_S1_FIELD_SIZE      (0UL)  /**< Packet S1 field size in bits. */
#define PACKET_S0_FIELD_SIZE      (0UL)  /**< Packet S0 field size in bits. */
#define PACKET_LENGTH_FIELD_SIZE  (0UL)  /**< Packet length field size in bits. */

static void radio_configure()
{
    // Radio config
    NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->FREQUENCY = 7UL;  // Frequency bin 7, 2407MHz
    NRF_RADIO->MODE      = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);

    // Radio address config
    NRF_RADIO->PREFIX0 =
        ((uint32_t)(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
      | ((uint32_t)(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
      | ((uint32_t)(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
      | ((uint32_t)(0xC0) << 0); // Prefix byte of address 0 converted to nRF24L series format

    NRF_RADIO->PREFIX1 =
        ((uint32_t)(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
      | ((uint32_t)(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
      | ((uint32_t)(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format

    NRF_RADIO->BASE0 = 001234567UL;  // Base address for prefix 0 converted to nRF24L series format
    NRF_RADIO->BASE1 = 0x89ABCDEFUL;  // Base address for prefix 1-7 converted to nRF24L series format

    NRF_RADIO->TXADDRESS   = 0x00UL;  // Set device address 0 to use when transmitting
    NRF_RADIO->RXADDRESSES = RADIO_RXADDRESSES_ADDR0_Msk;  // Enable device address 0 to use to select which addresses to receive

    // Packet configuration
    NRF_RADIO->PCNF0 = (PACKET_S1_FIELD_SIZE     << RADIO_PCNF0_S1LEN_Pos) |
                       (PACKET_S0_FIELD_SIZE     << RADIO_PCNF0_S0LEN_Pos) |
                       (PACKET_LENGTH_FIELD_SIZE << RADIO_PCNF0_LFLEN_Pos);

    // Packet configuration
    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big       << RADIO_PCNF1_ENDIAN_Pos)  |
                       (PACKET_BASE_ADDRESS_LENGTH   << RADIO_PCNF1_BALEN_Pos)   |
                       (PACKET_STATIC_LENGTH         << RADIO_PCNF1_STATLEN_Pos) |
                       (PACKET_PAYLOAD_MAXSIZE       << RADIO_PCNF1_MAXLEN_Pos);

    // CRC Config
    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
    if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFFFUL;   // Initial value
        NRF_RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16 + x^12^x^5 + 1
    }
    else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFUL;   // Initial value
        NRF_RADIO->CRCPOLY = 0x107UL;  // CRC poly: x^8 + x^2^x^1 + 1
    }
}

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

        void radio::enable_transmit() {
            NRF_RADIO->EVENTS_READY = 0U;
            NRF_RADIO->TASKS_TXEN = 1U;

            tos::arm::data.ready.down();
        }

        void radio::transmit(uint32_t data)
        {
            enable_transmit();
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);

            NRF_RADIO->EVENTS_END  = 0U;
            NRF_RADIO->TASKS_START = 1U;

            tos::arm::data.end.down();

            disable_radio();
        }

        void radio::enable_receive() {
            NRF_RADIO->EVENTS_READY = 0U;
            NRF_RADIO->TASKS_RXEN = 1U;

            tos::arm::data.ready.down();
        }

        uint32_t radio::receive()
        {
            volatile uint32_t data{};
            NRF_RADIO->PACKETPTR = reinterpret_cast<uint32_t>(&data);

            uint32_t result = 0;

            enable_receive();

            NRF_RADIO->EVENTS_END = 0U;
            NRF_RADIO->TASKS_START = 1U;

            tos::arm::data.end.down();
            //tos::arm::data.end.down();

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