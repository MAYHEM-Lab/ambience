#include "tos/ft.hpp"
#include "tos/meta/offsetof.hpp"
#include <memory>
#include <new>
#include <stm32_hal/gpio.hpp>
#include <stm32_hal/rcc.hpp>
#include <stm32_hal/rcc_ex.hpp>
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_cortex.h>
#include <tos/debug/log.hpp>
#include <tos/periph/stm32f7_eth/driver.hpp>

namespace tos::periph::stm32f7 {
namespace {
ethernet* eth_instance;
}
} // namespace tos::periph::stm32f7
extern "C" {
void ETH_IRQHandler() {
    tos::periph::stm32f7::eth_instance->irq();
}
}

namespace tos::periph::stm32f7 {
void ethernet::irq() {
    HAL_ETH_IRQHandler(&m_handle);
}

std::unique_ptr<ethernet> ethernet::open(const mac_addr_t& mac_addr) {
    __ETH_CLK_ENABLE();

    /**ETH GPIO Configuration
    PG14     ------> ETH_TXD1
    PG13     ------> ETH_TXD0
    PG11     ------> ETH_TX_EN
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PC4     ------> ETH_RXD0
    PA2     ------> ETH_MDIO
    PC5     ------> ETH_RXD1
    PA7     ------> ETH_CRS_DV
    */

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    auto res = new (std::nothrow) ethernet{};
    eth_instance = res;

    Assert(res);

    res->m_addr = mac_addr;

    auto& handle = res->m_handle;
    handle.Instance = ETH;

    handle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    handle.Init.PhyAddress = 0;
    handle.Init.MACAddr = res->m_addr.addr.data();
    handle.Init.RxMode = ETH_RXINTERRUPT_MODE;
    handle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    handle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    handle.Init.Speed = ETH_SPEED_100M;
    tos::debug::log("Calling init");

    auto status = HAL_ETH_Init(&handle);
    tos::debug::log(status);

    handle.RxCpltCallback = &rx_cplt_trampoline;
    handle.TxCpltCallback = &tx_cplt_trampoline;
    handle.MspDeInitCallback = [](auto) {};
    handle.MspInitCallback = [](auto) {};
    handle.DMAErrorCallback = [](auto) {};

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    HAL_ETH_DMATxDescListInit(
        &handle, res->DMATxDscrTab, &res->Tx_Buff[0][0], ETH_TXBUFNB);
    HAL_ETH_DMARxDescListInit(
        &handle, res->DMARxDscrTab, &res->Rx_Buff[0][0], ETH_RXBUFNB);

    HAL_ETH_Start(&handle);

    uint32_t regvalue;
    /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
    /* Read Register Configuration */
    HAL_ETH_ReadPHYRegister(&handle, PHY_MICR, &regvalue);

    regvalue |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);

    /* Enable Interrupts */
    HAL_ETH_WritePHYRegister(&handle, PHY_MICR, regvalue);

    /* Read Register Configuration */
    HAL_ETH_ReadPHYRegister(&handle, PHY_MISR, &regvalue);

    regvalue |= PHY_MISR_LINK_INT_EN;

    /* Enable Interrupt on change of link status */
    HAL_ETH_WritePHYRegister(&handle, PHY_MISR, regvalue);

    res->add();
    set_default(*res);
    res->up();

    return std::unique_ptr<ethernet>(res);
}

void ethernet::tx_cplt_trampoline(ETH_HandleTypeDef* handle) {
    meta::reverse_member<&ethernet::m_handle>(*handle).tx_cplt();
}

void ethernet::rx_cplt_trampoline(ETH_HandleTypeDef* handle) {
    meta::reverse_member<&ethernet::m_handle>(*handle).rx_cplt();
}

void ethernet::rx_cplt() {
    m_rx_sem.up_isr();
}

void ethernet::tx_cplt() {
}

pbuf* ethernet::low_level_input() {
    struct pbuf* p = NULL;
    struct pbuf* q;
    uint16_t len = 0;
    uint8_t* buffer;
    __IO ETH_DMADescTypeDef* dmarxdesc;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i = 0;


    /* get received frame */
    if (HAL_ETH_GetReceivedFrame_IT(&m_handle) != HAL_OK)
        return NULL;

    /* Obtain the size of the packet and put it into the "len" variable. */
    len = m_handle.RxFrameInfos.length;
    buffer = (uint8_t*)m_handle.RxFrameInfos.buffer;

    if (len > 0) {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

    if (p != NULL) {
        dmarxdesc = m_handle.RxFrameInfos.FSRxDesc;
        bufferoffset = 0;
        for (q = p; q != NULL; q = q->next) {
            byteslefttocopy = q->len;
            payloadoffset = 0;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx
             * buffer size*/
            while ((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE) {
                /* Copy data to pbuf */
                memcpy((uint8_t*)((uint8_t*)q->payload + payloadoffset),
                       (uint8_t*)((uint8_t*)buffer + bufferoffset),
                       (ETH_RX_BUF_SIZE - bufferoffset));

                /* Point to next descriptor */
                dmarxdesc = (ETH_DMADescTypeDef*)(dmarxdesc->Buffer2NextDescAddr);
                buffer = (uint8_t*)(dmarxdesc->Buffer1Addr);

                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset = 0;
            }
            /* Copy remaining data in pbuf */
            memcpy((uint8_t*)((uint8_t*)q->payload + payloadoffset),
                   (uint8_t*)((uint8_t*)buffer + bufferoffset),
                   byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }

        /* Release descriptors to DMA */
        /* Point to first descriptor */
        dmarxdesc = m_handle.RxFrameInfos.FSRxDesc;
        /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
        for (i = 0; i < m_handle.RxFrameInfos.SegCount; i++) {
            dmarxdesc->Status |= ETH_DMARXDESC_OWN;
            dmarxdesc = (ETH_DMADescTypeDef*)(dmarxdesc->Buffer2NextDescAddr);
        }

        /* Clear Segment_Count */
        m_handle.RxFrameInfos.SegCount = 0;
    }

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((m_handle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET) {
        /* Clear RBUS ETHERNET DMA flag */
        m_handle.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        m_handle.Instance->DMARPDR = 0;
    }
    return p;
}

err_t ethernet::init() {
    LOG("In init");
    pre_init();
    m_if.hostname = "ambience";
    m_if.flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
    m_if.name[1] = m_if.name[0] = 'm';
    m_if.num = 0;
    m_if.mtu = 1500;
    LOG("MTU", m_if.mtu);
    m_if.hwaddr_len = 6;
    memcpy(m_if.hwaddr, m_addr.addr.data(), 6);
    auto& t = launch(tos::alloc_stack,
                     [this] { read_thread(tos::cancellation_token::system()); });
    set_name(t, "LWIP Read Thread");
    return ERR_OK;
}

void ethernet::read_thread(tos::cancellation_token& tok) {
    bool printed = false;

    while (!tok.is_cancelled()) {
        m_rx_sem.down();
        receive_ctr.inc();
        while (true) {
            auto p = low_level_input();
            if (p == nullptr) {
                break;
            }
            // LOG("Got packet", p->tot_len);
            m_if.input(p, &m_if);
            if (!printed && dhcp_supplied_address(&m_if)) {
                printed = true;
                auto addr = tos::lwip::convert_to_tos(m_if.ip_addr);
                tos::debug::log("Got addr!",
                                (int)addr.addr[0],
                                (int)addr.addr[1],
                                (int)addr.addr[2],
                                (int)addr.addr[3]);
            }
        }
    }
}

err_t ethernet::link_output(struct pbuf* p) {
    output_ctr.inc();

    err_t errval;
    struct pbuf* q;
    uint8_t* buffer = (uint8_t*)(m_handle.TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef* DmaTxDesc;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;
    DmaTxDesc = m_handle.TxDesc;
    bufferoffset = 0;

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next) {
        /* Is this buffer available? If not, goto error */
        if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
            errval = ERR_USE;
            goto error;
        }

        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE) {
            /* Copy data to Tx buffer*/
            memcpy((uint8_t*)((uint8_t*)buffer + bufferoffset),
                   (uint8_t*)((uint8_t*)q->payload + payloadoffset),
                   (ETH_TX_BUF_SIZE - bufferoffset));

            /* Point to next descriptor */
            DmaTxDesc = (ETH_DMADescTypeDef*)(DmaTxDesc->Buffer2NextDescAddr);

            /* Check if the buffer is available */
            if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
                errval = ERR_USE;
                goto error;
            }

            buffer = (uint8_t*)(DmaTxDesc->Buffer1Addr);

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        memcpy((uint8_t*)((uint8_t*)buffer + bufferoffset),
               (uint8_t*)((uint8_t*)q->payload + payloadoffset),
               byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

    /* Prepare transmit descriptors to give to DMA */
    HAL_ETH_TransmitFrame(&m_handle, framelength);

    errval = ERR_OK;

error:

    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to
     * resume transmission */
    if ((m_handle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET) {
        /* Clear TUS ETHERNET DMA flag */
        m_handle.Instance->DMASR = ETH_DMASR_TUS;

        /* Resume DMA transmission*/
        m_handle.Instance->DMATPDR = 0;
    }

    // tos::debug::log("Output", p->len, errval);

    return errval;
}
} // namespace tos::periph::stm32f7