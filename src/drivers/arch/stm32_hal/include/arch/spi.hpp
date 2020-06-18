//
// Created by fatih on 9/11/19.
//

#pragma once

#include "gpio.hpp"

#include <arch/detail/afio.hpp>
#include <optional>
#include <stm32_hal/dma.hpp>
#include <stm32_hal/rcc.hpp>
#include <stm32_hal/spi.hpp>
#include <tos/debug/debug.hpp>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

namespace tos::stm32 {
namespace detail {
struct spi_def {
    uint32_t spi;
    IRQn_Type irq;
    void (*rcc)();
};

inline spi_def spis[] = {
    {SPI1_BASE, SPI1_IRQn, [] { __HAL_RCC_SPI1_CLK_ENABLE(); }},
    {SPI2_BASE, SPI2_IRQn, [] { __HAL_RCC_SPI2_CLK_ENABLE(); }},
#if defined(SPI3)
    {SPI3_BASE, SPI3_IRQn, [] { __HAL_RCC_SPI3_CLK_ENABLE(); }},
#endif
};
} // namespace detail

enum class spi_errors
{

};

class spi
    : public tracked_driver<spi, std::size(detail::spis)>
    , public self_pointing<spi>
    , public non_copy_movable {
public:
    using gpio_type = gpio;

    spi(detail::spi_def& spi,
        gpio::pin_type sck,
        std::optional<gpio::pin_type> miso,
        std::optional<gpio::pin_type> mosi);

    ~spi();

    expected<void, spi_errors> write(tos::span<const char> data);
    expected<void, spi_errors> write(tos::span<const uint8_t> data);

    expected<void, spi_errors> exchange(tos::span<char> rx, tos::span<const char> tx);
    expected<void, spi_errors> exchange(tos::span<uint8_t> rx,
                                        tos::span<const uint8_t> tx);

    void isr();
    void tx_done_isr();

private:
    tos::semaphore m_busy_sem{0};

    SPI_HandleTypeDef m_handle;
};
} // namespace tos::stm32

// Implementation

namespace tos::stm32 {
inline spi::spi(detail::spi_def& spi,
                gpio::pin_type sck,
                std::optional<gpio::pin_type> miso,
                std::optional<gpio::pin_type> mosi)
    : tracked_driver(std::distance(&detail::spis[0], &spi))
    , m_handle{} {
    m_handle.Instance = reinterpret_cast<decltype(m_handle.Instance)>(spi.spi);

    SPI_InitTypeDef& spi_init = m_handle.Init;
    spi_init.Mode = SPI_MODE_MASTER;
    spi_init.Direction = SPI_DIRECTION_2LINES;
    spi_init.DataSize = SPI_DATASIZE_8BIT;
    spi_init.CLKPolarity = SPI_POLARITY_LOW;
    spi_init.CLKPhase = SPI_PHASE_1EDGE;
    spi_init.NSS = SPI_NSS_SOFT;
    spi_init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    spi_init.FirstBit = SPI_FIRSTBIT_MSB;
    spi_init.TIMode = SPI_TIMODE_DISABLE;
    spi_init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_init.CRCPolynomial = 7;

#if !defined(STM32F1)
    auto [sck_afio, miso_afio, mosi_afio] =
        detail::afio::get_spi_afio(spi.spi, sck, miso, mosi);
#endif
    {
        enable_rcc(sck.port);
        GPIO_InitTypeDef init{};
        init.Pin = sck.pin;
        init.Mode = GPIO_MODE_AF_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = detail::gpio_speed::highest();
#if !defined(STM32F1)
        init.Alternate = sck_afio;
#endif
        HAL_GPIO_Init(sck.port, &init);
    }

    if (miso) {
        enable_rcc(miso->port);
        GPIO_InitTypeDef init{};
        init.Pin = miso->pin;
        init.Mode = GPIO_MODE_AF_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = detail::gpio_speed::highest();
#if !defined(STM32F1)
        init.Alternate = miso_afio;
#endif
        HAL_GPIO_Init(miso->port, &init);
    }

    if (mosi) {
        enable_rcc(mosi->port);
        GPIO_InitTypeDef init{};
        init.Pin = mosi->pin;
        init.Mode = GPIO_MODE_AF_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = detail::gpio_speed::highest();
#if !defined(STM32F1)
        init.Alternate = mosi_afio;
#endif
        HAL_GPIO_Init(mosi->port, &init);
    }

    spi.rcc();

    if (HAL_SPI_Init(&m_handle) != HAL_OK) {
        tos::debug::panic("Driver initialization failure");
    }

    HAL_NVIC_SetPriority(spi.irq, 0, 0);
    HAL_NVIC_EnableIRQ(spi.irq);
}

inline spi::~spi() {
    HAL_SPI_DeInit(&m_handle);
}

inline expected<void, spi_errors> spi::write(tos::span<const char> data) {
    auto hal_ptr = reinterpret_cast<uint8_t*>(const_cast<char*>(data.data()));
    auto res = HAL_SPI_Transmit_IT(&m_handle, hal_ptr, data.size());
    if (res != HAL_OK) {
        return unexpected(spi_errors{});
    }
    tos::kern::busy();
    m_busy_sem.down();
    tos::kern::unbusy();
    while ((m_handle.Instance->SR & SPI_FLAG_BSY) == SPI_FLAG_BSY)
        ;
    return {};
}

inline expected<void, spi_errors> spi::write(tos::span<const uint8_t> data) {
    return write(raw_cast<const char>(data));
}

inline expected<void, spi_errors> spi::exchange(tos::span<char> rx,
                                                tos::span<const char> tx) {
    auto rx_hal_ptr = reinterpret_cast<uint8_t*>(rx.data());
    auto tx_hal_ptr = reinterpret_cast<uint8_t*>(const_cast<char*>(tx.data()));
    // HAL_SPIEx_FlushRxFifo(&m_handle);

    auto res = HAL_SPI_TransmitReceive_IT(&m_handle, tx_hal_ptr, rx_hal_ptr, rx.size());
    if (res != HAL_OK) {
        return unexpected(spi_errors{});
    }
    tos::kern::busy();
    m_busy_sem.down();
    tos::kern::unbusy();
    while ((m_handle.Instance->SR & SPI_FLAG_BSY) == SPI_FLAG_BSY)
        ;
    return {};
}

inline expected<void, spi_errors> spi::exchange(tos::span<uint8_t> rx,
                                                tos::span<const uint8_t> tx) {
    return exchange(raw_cast<char>(rx), raw_cast<const char>(tx));
}

inline void spi::isr() {
    HAL_SPI_IRQHandler(&m_handle);
}

inline void spi::tx_done_isr() {
    m_busy_sem.up_isr();
}
} // namespace tos::stm32