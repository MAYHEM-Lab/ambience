#pragma once

#include <common/driver_base.hpp>
#include <cstdint>
#include <stm32f7xx_hal_conf.h>
#include <stm32f7xx_hal_dma2d.h>
#include <tos/semaphore.hpp>

namespace tos::stm32::f7 {
class dma2d : public tracked_driver<dma2d, 1> {
public:
    dma2d();

    void fill_rect(void* data, uint32_t color, int width, int height);

    void irq();

    auto native_handle() {
        return &m_dma;
    }

private:
    semaphore m_sem{0};
    DMA2D_HandleTypeDef m_dma{};
};
} // namespace tos::stm32::f7