#pragma once

/**
 * This header is used to include the correct CMSIS device header.
 */

#if defined(TOS_PLATFORM_nrf52)
#include <nrf.h>
#elif defined(TOS_PLATFORM_cc32xx)
#define INT_IRQn_OFFSET 16

typedef enum IRQn
{
    /* =======================================  ARM Cortex-M4 Specific Interrupt Numbers
       ======================================== */

    Reset_IRQn = -15, /*!< -15  Reset Vector, invoked on Power up and warm reset */
    NonMaskableInt_IRQn =
        -14, /*!< -14  Non maskable Interrupt, cannot be stopped or preempted */
    HardFault_IRQn = -13,        /*!< -13  Hard Fault, all classes of Fault        */
    MemoryManagement_IRQn = -12, /*!< -12  Memory Management, MPU mismatch, including
                                    Access Violation and No Match */
    BusFault_IRQn = -11, /*!< -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other
                            address/memory related Fault */
    UsageFault_IRQn =
        -10, /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition */
    SVCall_IRQn = -5,       /*!< -5 System Service Call via SVC instruction       */
    DebugMonitor_IRQn = -4, /*!< -4 Debug Monitor */
    PendSV_IRQn = -2,       /*!< -2 Pendable request for system service       */
    SysTick_IRQn = -1,      /*!< -1 System Tick Timer      */

    /* ===========================================  CC3220SF Specific Interrupt Numbers
       ========================================= */
    INT_GPIOA0_IRQn = 16 - INT_IRQn_OFFSET,   // GPIO Port S0
    INT_GPIOA1_IRQn = 17 - INT_IRQn_OFFSET,   // GPIO Port S1
    INT_GPIOA2_IRQn = 18 - INT_IRQn_OFFSET,   // GPIO Port S2
    INT_GPIOA3_IRQn = 19 - INT_IRQn_OFFSET,   // GPIO Port S3
    INT_UARTA0_IRQn = 21 - INT_IRQn_OFFSET,   // UART0 Rx and Tx
    INT_UARTA1_IRQn = 22 - INT_IRQn_OFFSET,   // UART1 Rx and Tx
    INT_I2CA0_IRQn = 24 - INT_IRQn_OFFSET,    // I2C controller
    INT_ADCCH0_IRQn = 30 - INT_IRQn_OFFSET,   // ADC Sequence 0
    INT_ADCCH1_IRQn = 31 - INT_IRQn_OFFSET,   // ADC Sequence 1
    INT_ADCCH2_IRQn = 32 - INT_IRQn_OFFSET,   // ADC Sequence 2
    INT_ADCCH3_IRQn = 33 - INT_IRQn_OFFSET,   // ADC Sequence 3
    INT_WDT_IRQn = 34 - INT_IRQn_OFFSET,      // Watchdog Timer0
    INT_TIMERA0A_IRQn = 35 - INT_IRQn_OFFSET, // Timer 0 subtimer A
    INT_TIMERA0B_IRQn = 36 - INT_IRQn_OFFSET, // Timer 0 subtimer B
    INT_TIMERA1A_IRQn = 37 - INT_IRQn_OFFSET, // Timer 1 subtimer A
    INT_TIMERA1B_IRQn = 38 - INT_IRQn_OFFSET, // Timer 1 subtimer B
    INT_TIMERA2A_IRQn = 39 - INT_IRQn_OFFSET, // Timer 2 subtimer A
    INT_TIMERA2B_IRQn = 40 - INT_IRQn_OFFSET, // Timer 2 subtimer B
    INT_FLASH_IRQn = 45 - INT_IRQn_OFFSET,    // FLASH Control
    INT_TIMERA3A_IRQn = 51 - INT_IRQn_OFFSET, // Timer 3 subtimer A
    INT_TIMERA3B_IRQn = 52 - INT_IRQn_OFFSET, // Timer 3 subtimer B
    INT_UDMA_IRQn = 62 - INT_IRQn_OFFSET,     // uDMA controller
    INT_UDMAERR_IRQn = 63 - INT_IRQn_OFFSET,  // uDMA Error
    INT_SHA_IRQn = 164 - INT_IRQn_OFFSET,     // SHA
    INT_AES_IRQn = 167 - INT_IRQn_OFFSET,     // AES
    INT_DES_IRQn = 169 - INT_IRQn_OFFSET,     // DES
    INT_MMCHS_IRQn = 175 - INT_IRQn_OFFSET,   // SDIO
    INT_I2S_IRQn = 177 - INT_IRQn_OFFSET,     // McAPS
    INT_CAMERA_IRQn = 179 - INT_IRQn_OFFSET,  // Camera
    INT_NWPIC_IRQn = 187 - INT_IRQn_OFFSET,   // Interprocessor communication
    INT_PRCM_IRQn = 188 - INT_IRQn_OFFSET,    // Power, Reset and Clock Module
    INT_SSPI_IRQn = 191 - INT_IRQn_OFFSET,    // Shared SPI
    INT_GSPI_IRQn = 192 - INT_IRQn_OFFSET,    // Generic SPI
    INT_LSPI_IRQn = 193 - INT_IRQn_OFFSET     // Link SPI
} IRQn_Type;


/* ===========================================================================================================================
 */
/* ================                           Processor and Core Peripheral Section
 * ================ */
/* ===========================================================================================================================
 */

/* ===========================  Configuration of the Arm Cortex-M4 Processor and Core
 * Peripherals  =========================== */
#define __CM4_REV 0x0201         /*!< Core Revision r2p1 */
/* ToDo: define the correct core features for the CC3220SF */
#define __MPU_PRESENT 0          /*!< Set to 1 if MPU is present */
#define __VTOR_PRESENT 1         /*!< Set to 1 if VTOR is present */
#define __NVIC_PRIO_BITS 3       /*!< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig 0 /*!< Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT 0          /*!< Set to 1 if FPU is present */
#define __FPU_DP                                                                         \
    0 /*!< Set to 1 if FPU is double precision FPU (default is single precision FPU) */
#define __ICACHE_PRESENT 0 /*!< Set to 1 if I-Cache is present */
#define __DCACHE_PRESENT 0 /*!< Set to 1 if D-Cache is present */
#define __DTCM_PRESENT 0   /*!< Set to 1 if DTCM is present */


/** @} */ /* End of group Configuration_of_CMSIS */

#include <core_cm4.h> /*!< Arm Cortex-M4 processor and core peripherals */
#elif defined(TOS_PLATFORM_stm32_hal)
#include <stm32_hal/hal.hpp>
#endif