//
// Created by fatih on 10/25/18.
//

#include <libopencm3/stm32/rcc.h>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/ft.hpp>
#include <tos_arch.hpp>
#include <libopencm3/stm32/flash.h>

void tos_main();

int main()
{
#if defined(STM32F1)
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
#elif defined(STM32L0)
#elif defined(STM32F4)
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_120MHZ]);
#elif defined(STM32L4)
	rcc_osc_on(RCC_HSI16);
    rcc_wait_for_osc_ready(RCC_HSI16);

	flash_prefetch_enable();
	flash_set_ws(4);
	flash_dcache_enable();
	flash_icache_enable();
	/* 16MHz / 4 = > 4 * 40 = 160MHz VCO => 80MHz main pll  */
	rcc_set_main_pll(RCC_PLLCFGR_PLLSRC_HSI16, 4, 40,
			0, 0, RCC_PLLCFGR_PLLR_DIV2);
    rcc_osc_on(RCC_PLL);
    while (rcc_is_osc_ready(RCC_PLL));
    //rcc_wait_for_osc_ready(RCC_PLL);

    rcc_periph_clock_enable(RCC_SYSCFG);

    rcc_set_sysclk_source(RCC_CFGR_SW_PLL); /* careful with the param here! */
    rcc_wait_for_sysclk_status(RCC_PLL);
    /* FIXME - eventually handled internally */
    rcc_ahb_frequency = 80e6;
    rcc_apb1_frequency = 80e6;
    rcc_apb2_frequency = 80e6;
#endif

    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) {
            tos_force_reset();
        }
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __WFI();// power_down(SLEEP_MODE_IDLE);
    }
}