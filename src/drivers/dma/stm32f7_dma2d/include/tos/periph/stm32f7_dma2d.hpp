#pragma once

#include <cstdint>
#include <stm32f7xx_hal_conf.h>
#include <stm32f7xx_hal_dma2d.h>

namespace tos::stm32::f7 {
class dma2d {
public:
    dma2d();

    void fill_rect(void* data, uint32_t color, int width, int height);

private:

    DMA2D_HandleTypeDef m_dma{};
};
}