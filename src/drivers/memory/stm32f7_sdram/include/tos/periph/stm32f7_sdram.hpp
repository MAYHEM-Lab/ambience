#pragma once

#include <stm32f7xx_hal_sdram.h>

namespace tos::stm32::f7 {
/**
 * This driver currently only works with the SDRAM setup on the STM32F7 Discovery board.
 */
class sdram {
public:
    sdram();

    void* bank_base() const;

private:
    void init_sequence();
    void setup_gpio();

    SDRAM_HandleTypeDef m_sdram{};
    FMC_SDRAM_TimingTypeDef m_timing{};
    FMC_SDRAM_CommandTypeDef m_command{};
};
}