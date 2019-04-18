//
// Created by fatih on 1/9/19.
//

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>
#include <tos/compiler.hpp>
#include <cstddef>
#include <cstdlib>

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

void *__dso_handle;

void* tos_stack_alloc(size_t sz)
{
    return malloc(sz);
}

void tos_stack_free(void* ptr)
{
    return free(ptr);
}
}

static bool tried_bkpt = false;
void hard_fault_handler()
{
    if (!tried_bkpt)
    {
        tried_bkpt = true;
        asm("bkpt");
    }
    else
    {
        tos_force_reset();
        while(true)
        {
        }
    }

}