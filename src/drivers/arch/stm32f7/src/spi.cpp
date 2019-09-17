//
// Created by fatih on 9/11/19.
//

#include <arch/spi.hpp>

using namespace tos::stm32;

extern "C" {
void SPI1_IRQHandler() {
    tos::stm32::spi::get(0)->isr();
}
void SPI2_IRQHandler() {
    tos::stm32::spi::get(1)->isr();
}
void SPI3_IRQHandler() {
    tos::stm32::spi::get(2)->isr();
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* spi_handle)
{
    HAL_SPI_TxCpltCallback(spi_handle);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* spi_handle)
{
    HAL_SPI_TxCpltCallback(spi_handle);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* spi_handle) {
    switch (uintptr_t(spi_handle->Instance)) {
    case SPI1_BASE:
        spi::get(0)->tx_done_isr();
        return;
    case SPI2_BASE:
        spi::get(1)->tx_done_isr();
        return;
#ifdef SPI3_BASE
    case SPI3_BASE:
        spi::get(2)->tx_done_isr();
        return;
#endif
    }
}
}