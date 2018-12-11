//
// Created by fatih on 10/25/18.
//

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/ft.hpp>
#include <tos_arch.hpp>
#include <libopencmsis/core_cm3.h>

#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)   /*!< AIRCR Key for write access   */
#define NVIC_SYSRESETREQ            2         /*!< System Reset Request         */

#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")

static __INLINE NORETURN void NVIC_SystemReset()
{
    SCB->AIRCR  = (NVIC_AIRCR_VECTKEY | (SCB->AIRCR & (0x700)) | (1<<NVIC_SYSRESETREQ)); /* Keep priority group unchanged */
    dsb();                                                                                 /* Ensure completion of memory access */
    while(1);                                                                                /* wait until reset */
}

extern "C"
{
    void NORETURN tos_force_reset()
    {
        NVIC_SystemReset();
    }

extern "C" void *__dso_handle;
void *__dso_handle = 0;

void* tos_stack_alloc(size_t sz)
{
    return malloc(sz);
}

void tos_stack_free(void* ptr)
{
    return free(ptr);
}
}

void tos_main();

int main()
{
#ifdef STM32F1
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
#else
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_120MHZ]);
#endif

    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        //if (res == tos::exit_reason::restart) NVIC_SystemReset();// reboot();
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __asm__("wfe");// power_down(SLEEP_MODE_IDLE);
    }
}