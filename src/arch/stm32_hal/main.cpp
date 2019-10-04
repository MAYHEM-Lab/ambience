#include <tos/compiler.hpp>
#include <tos/ft.hpp>
#include <tos/scheduler.hpp>

extern "C" {
int __dso_handle;
}
extern "C" void _init() {
}

extern void tos_main();

extern "C" void SysTick_Handler() {
    HAL_IncTick();
}

void Error_Handler() {
    __BKPT(0);
}

namespace tos {
namespace stm32 {
uint32_t apb1_clock = -1;
uint32_t ahb_clock = -1;
} // namespace stm32
} // namespace tos

#if defined(STM32F7)
void SystemClock_Config() {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
        Error_Handler();
    }

    tos::stm32::apb1_clock = 54'000'000;
    tos::stm32::ahb_clock = 108'000'000;
}
#elif defined(STM32L0)
void SystemClock_Config() {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_OscInitTypeDef RCC_OscInitLSI;

    RCC_OscInitLSI.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitLSI.LSIState = RCC_LSI_ON;

    if (HAL_RCC_OscConfig(&RCC_OscInitLSI) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_RTC;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }

    tos::stm32::apb1_clock = 16'000'000;
    tos::stm32::ahb_clock = 32'000'000;
}
#elif defined(STM32L4)
void SystemClock_Config() {
    tos::stm32::apb1_clock = 2'000'000;
    tos::stm32::ahb_clock = 4'000'000;
}
#endif

namespace {
bool tried_bkpt = false;
}
extern "C" void HardFault_Handler() {
    if (!tried_bkpt) {
        tried_bkpt = true;
        __BKPT(0);
    } else {
        tos_force_reset();
        TOS_UNREACHABLE();
    }
}

int main() {
    HAL_Init();
    SystemClock_Config();

    // Interrupts are already enabled:
    tos::kern::enable_interrupts();
    // tos::kern::detail::disable_depth--;

    tos_main();

    while (true) {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) {
            tos_force_reset();
        }
        if (res == tos::exit_reason::power_down) {
            __WFI();
        }
        if (res == tos::exit_reason::idle) {
            __WFI();
        }
        if (res == tos::exit_reason::yield) {
            // Do nothing
        }
    }
}
