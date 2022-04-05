#pragma once

#include "common/driver_base.hpp"
#include "lwip/pbuf.h"
#include "tos/meta/offsetof.hpp"
#include "tos/self_pointing.hpp"
#include "tos/utility.hpp"
#include <common/inet/tcp_ip.hpp>
#include <cstdint>
#include <stm32_hal/eth.hpp>
#include <tos/expected.hpp>
#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/semaphore.hpp>
#include <tos/debug/trace/metrics/counter.hpp>

namespace tos::periph::stm32f7 {
struct ethernet
    : self_pointing<ethernet>
    , non_copy_movable
    , public tos::lwip::netif_base<ethernet> {
    ethernet()
        : m_handle {
    }
    {
    }

    mac_addr_t address() const {
        return m_addr;
    }

    static std::unique_ptr<ethernet> open(const mac_addr_t& mac_addr);

    void irq();


    alignas(alignof(uintptr_t)) ETH_DMADescTypeDef DMARxDscrTab[ETH_RXBUFNB];
    alignas(alignof(uintptr_t)) ETH_DMADescTypeDef DMATxDscrTab[ETH_TXBUFNB];
    alignas(alignof(uintptr_t)) uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
    alignas(alignof(uintptr_t)) uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

    static void rx_cplt_trampoline(ETH_HandleTypeDef*);
    static void tx_cplt_trampoline(ETH_HandleTypeDef*);
    void rx_cplt();
    void tx_cplt();

    err_t init();
    void read_thread(tos::cancellation_token& tok);
    err_t link_output(pbuf* p);
    pbuf* low_level_input();
    
    ETH_HandleTypeDef m_handle;
    mac_addr_t m_addr;
    semaphore m_rx_sem{0};

    trace::basic_counter output_ctr;
    trace::basic_counter receive_ctr;
};
} // namespace tos::periph::stm32f7