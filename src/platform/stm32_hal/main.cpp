#include <stm32_hal/flash.hpp>
#include <stm32_hal/rcc.hpp>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/platform.hpp>
#include <tos/interrupt.hpp>
#include <tos/arm/exception.hpp>

extern "C" {
void* __dso_handle;
void _fini() {
}
void _init() {
}
}

extern void tos_main();

extern "C" {
[[gnu::used]] void SysTick_Handler() {
    HAL_IncTick();
}
}

void Error_Handler() {
    __BKPT(0);
    tos::platform::force_reset();
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
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
        Error_Handler();
    }

    tos::stm32::apb1_clock = 54'000'000;
    tos::stm32::ahb_clock = 108'000'000;

    SCB_EnableICache();
    SCB_EnableDCache();
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
#elif defined(STM32L475xx)
void SystemClock_Config() {
    /*tos::stm32::apb1_clock = 2'000'000;
    tos::stm32::ahb_clock = 4'000'000;
    return;*/
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        /* Initialization Error */
        while (1)
            ;
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        /* Initialization Error */
        while (1)
            ;
    }

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while (1)
            ;
    }

    tos::stm32::apb1_clock = 40'000'000;
    tos::stm32::ahb_clock = 80'000'000;
}
#elif defined(STM32L412xx)
void SystemClock_Config() {
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};


    /* MSI is enabled after System reset, activate PLL with MSI as source */

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;

    RCC_OscInitStruct.MSIState = RCC_MSI_ON;

    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;

    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;

    RCC_OscInitStruct.PLL.PLLM = 1;

    RCC_OscInitStruct.PLL.PLLN = 40;

    RCC_OscInitStruct.PLL.PLLR = 2;

    RCC_OscInitStruct.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)

    {

        /* Initialization Error */

        while (1)
            ;
    }


    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2

       clocks dividers */

    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)

    {

        /* Initialization Error */

        while (1)
            ;
    }

    RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    tos::stm32::apb1_clock = 40'000'000;
    tos::stm32::ahb_clock = 80'000'000;
}
#elif defined(STM32F1)
void SystemClock_Config() {
    RCC_ClkInitTypeDef rccClkInit;
    RCC_OscInitTypeDef rccOscInit;

    /*## STEP 1: Configure HSE and PLL #######################################*/
    rccOscInit.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    rccOscInit.HSEState = RCC_HSE_ON;
    rccOscInit.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    rccOscInit.PLL.PLLState = RCC_PLL_ON;
    rccOscInit.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    rccOscInit.PLL.PLLMUL = RCC_PLL_MUL9;
    auto osc_res = HAL_RCC_OscConfig(&rccOscInit);
    if (osc_res != HAL_OK) {
        Error_Handler();
    }

    /*## STEP 2: Configure SYSCLK, HCLK, PCLK1, and PCLK2 ####################*/
    rccClkInit.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                            RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    rccClkInit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    rccClkInit.AHBCLKDivider = RCC_SYSCLK_DIV1;
    rccClkInit.APB2CLKDivider = RCC_HCLK_DIV1;
    rccClkInit.APB1CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&rccClkInit, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }

    tos::stm32::apb1_clock = 36'000'000;
    tos::stm32::ahb_clock = 72'000'000;
}
#endif

extern "C" void HardFault_Handler() {
    tos::arm::exception::hard_fault();
}

extern "C" void UsageFault_Handler() {
    tos::arm::exception::usage_fault();
}

extern "C" void MemManage_Handler() {
    tos::arm::exception::mem_fault();
}

extern "C" void BusFault_Handler() {
    tos::arm::exception::bus_fault();
}

extern "C" void SVC_Handler() {
    tos::arm::exception::out_svc_handler();
}

int main() {
    HAL_Init();
    SystemClock_Config();
    // NVIC_DisableIRQ(SysTick_IRQn);

    // Interrupts are already enabled:
    tos::kern::enable_interrupts();
    // tos::kern::detail::disable_depth--;
    HAL_NVIC_EnableIRQ(UsageFault_IRQn);
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;

    HAL_NVIC_EnableIRQ(BusFault_IRQn);
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;

    tos_main();

    while (true) {
        auto res = tos::global::sched.schedule();
        if (res == tos::exit_reason::restart) {
            tos::platform::force_reset();
        }
        if (res == tos::exit_reason::power_down) {
            HAL_SuspendTick();
            HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
            HAL_ResumeTick();

            NVIC_EnableIRQ(SysTick_IRQn);
            SystemClock_Config();
            NVIC_DisableIRQ(SysTick_IRQn);
            //__WFI();
        }
        if (res == tos::exit_reason::idle) {
            __WFI();
        }
        if (res == tos::exit_reason::yield) {
            // Do nothing
        }
    }
}
