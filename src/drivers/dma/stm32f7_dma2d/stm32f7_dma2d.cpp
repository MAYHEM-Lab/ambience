#include <tos/debug/panic.hpp>
#include <tos/periph/stm32f7_dma2d.hpp>
#include <stm32_hal/rcc.hpp>
#include <stm32f7xx_ll_dma2d.h>

namespace tos::stm32::f7 {
dma2d::dma2d() {
    m_dma.Instance = DMA2D;

    m_dma.Init.Mode = DMA2D_R2M;
    m_dma.Init.ColorMode = DMA2D_OUTPUT_RGB565;
    m_dma.Init.OutputOffset = 0;

    __HAL_RCC_DMA2D_CLK_ENABLE();

    auto res = HAL_DMA2D_Init(&m_dma);
    if (res != HAL_OK) {
        debug::panic("Couldn't init dma2d");
    }
}

void dma2d::fill_rect(void* data, uint32_t color, int width, int height) {
    HAL_DMA2D_Start(&m_dma, color, reinterpret_cast<uintptr_t>(data), width, height);
    HAL_DMA2D_PollForTransfer(&m_dma, 100);
}
}