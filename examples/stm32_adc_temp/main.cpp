//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <arch/stm32/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <common/dht22.hpp>
#include <common/hm-10.hpp>
#include <common/alarm.hpp>
#include <libopencm3/stm32/rtc.h>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
}

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

tos::event rtc_ev;
void rtc_isr(void)
{
    rtc_clear_flag(RTC_SEC);

    gpio_toggle(GPIOC, GPIO13);

    rtc_ev.fire_isr();
}

void temp_task()
{
    using namespace tos::tos_literals;

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(45_pin, tos::pin_mode::out);
    nvic_enable_irq(NVIC_RTC_IRQ);
    nvic_set_priority(NVIC_RTC_IRQ, 1);
    rtc_auto_awake(RCC_LSE, 0x7fff);
    rtc_interrupt_enable(RTC_SEC);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    auto hm10 = tos::open(tos::devs::hm10, &usart);

    tos::stm32::adc a { tos::stm32::detail::adcs };

    uint8_t x[1];
    x[0] = 16;
    a.set_channels(x);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    while (true)
    {
        using namespace std::chrono_literals;

        tos::println(hm10, int(a.read()));

        rtc_ev.wait();
        //alarm->sleep_for(100ms);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, temp_task);
}
