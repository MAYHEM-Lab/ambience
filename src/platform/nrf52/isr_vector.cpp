using ptr = void (*)();

struct [[gnu::packed]] isr_vec {
    void* stack_ptr;
    ptr exceptions[15];
    ptr list[112];
};

extern "C" {
extern char __StackTop;
void Reset_Handler();
void NMI_Handler();
void HardFault_Handler();
void MemoryManagement_Handler();
void BusFault_Handler();
void UsageFault_Handler();

void SVC_Handler();
void DebugMon_Handler();

void PendSV_Handler();
void SysTick_Handler();

void POWER_CLOCK_IRQHandler();
void RADIO_IRQHandler();
void UARTE0_UART0_IRQHandler();
void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler();
void SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler();
void NFCT_IRQHandler();
void GPIOTE_IRQHandler();
void SAADC_IRQHandler();
void TIMER0_IRQHandler();
void TIMER1_IRQHandler();

[[gnu::weak]] void POWER_CLOCK_IRQHandler() {
    while (true)
        ;
}

[[gnu::weak]] void NFCT_IRQHandler() {
    while (true)
        ;
}

[[gnu::weak]] void GPIOTE_IRQHandler() {
    while (true)
        ;
}

[[gnu::weak]] void SAADC_IRQHandler() {
    while (true)
        ;
}

[[gnu::section(".isr_vector"), gnu::used]] isr_vec __isr_vector_ = {
    .stack_ptr = &__StackTop,
    .exceptions =
        {
            Reset_Handler,
            NMI_Handler,
            HardFault_Handler,
            MemoryManagement_Handler,
            BusFault_Handler,
            UsageFault_Handler,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            SVC_Handler,
            DebugMon_Handler,
            nullptr,
            PendSV_Handler,
            SysTick_Handler,
        },
    .list = {
        POWER_CLOCK_IRQHandler,
        RADIO_IRQHandler,
        UARTE0_UART0_IRQHandler,
        SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler,
        SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler,
        NFCT_IRQHandler,
        GPIOTE_IRQHandler,
        SAADC_IRQHandler,
        TIMER0_IRQHandler,
        TIMER1_IRQHandler,
    }};
}
/*
 *     .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop
.long   Reset_Handler
.long   NMI_Handler
.long   HardFault_Handler
.long   MemoryManagement_Handler
.long   BusFault_Handler
.long   UsageFault_Handler
.long   0
.long   0
.long   0
.long   0
.long   SVC_Handler
.long   DebugMon_Handler
.long   0
.long   PendSV_Handler
.long   SysTick_Handler

.long   POWER_CLOCK_IRQHandler
.long   RADIO_IRQHandler
.long   UARTE0_UART0_IRQHandler
.long   SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
.long   SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
.long   NFCT_IRQHandler
.long   GPIOTE_IRQHandler
.long   SAADC_IRQHandler
.long   TIMER0_IRQHandler
.long   TIMER1_IRQHandler
.long   TIMER2_IRQHandler
.long   RTC0_IRQHandler
.long   TEMP_IRQHandler
.long   RNG_IRQHandler
.long   ECB_IRQHandler
.long   CCM_AAR_IRQHandler
.long   WDT_IRQHandler
.long   RTC1_IRQHandler
.long   QDEC_IRQHandler
.long   COMP_LPCOMP_IRQHandler
.long   SWI0_EGU0_IRQHandler
.long   SWI1_EGU1_IRQHandler
.long   SWI2_EGU2_IRQHandler
.long   SWI3_EGU3_IRQHandler
.long   SWI4_EGU4_IRQHandler
.long   SWI5_EGU5_IRQHandler
.long   TIMER3_IRQHandler
.long   TIMER4_IRQHandler
.long   PWM0_IRQHandler
.long   PDM_IRQHandler
.long   0
.long   0
.long   MWU_IRQHandler
.long   PWM1_IRQHandler
.long   PWM2_IRQHandler
.long   SPIM2_SPIS2_SPI2_IRQHandler
.long   RTC2_IRQHandler
.long   I2S_IRQHandler
.long   FPU_IRQHandler
.long   0
.long   0
.long   0
.long   CryptoCell_Handler
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0
.long   0

.size __isr_vector, . - __isr_vector
*/