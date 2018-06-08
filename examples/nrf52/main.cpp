//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>
#include <tos/compiler.hpp>
#include <drivers/common/gpio.hpp>
#include <drivers/arch/nrf52/gpio.hpp>
#include <drivers/arch/nrf52/usart.hpp>

tos::semaphore sem{0};

auto g = tos::open(tos::devs::gpio);

void led1_task()
{
    using namespace tos;
    g->write(17, digital::low);
    while (true)
    {
        g->write(17, digital::high);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }

        g->write(17, digital::low);
        for (int i = 0; i < 1000; ++i)
        {
            nrf_delay_us(100);
            tos::this_thread::yield();
        }
    }
}

void led2_task()
{
    using namespace tos;
    g->write(19, digital::low);
    while (true)
    {
        sem.down();
        g->write(19, digital::high);

        sem.down();
        g->write(19, digital::low);
    }
}

void tos_main()
{
    using namespace tos;
    using namespace tos_literals;
    constexpr nrfx_uarte_t uart { NRF_UARTE0, NRFX_UARTE0_INST_IDX };

    auto usconf = usart_config()
        .set(115200_baud_rate)
        .set(usart_parity::disabled)
        .set(usart_stop_bit::one);

    arm::uart u{usconf, 8, 6};

    nrfx_uarte_config_t conf{};

    conf.interrupt_priority = 7;
    conf.parity = NRF_UARTE_PARITY_EXCLUDED;
    conf.hwfc = NRF_UARTE_HWFC_DISABLED;
    conf.baudrate = NRF_UARTE_BAUDRATE_115200;
    conf.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrxd = 8;
    conf.pseltxd = 6;
    conf.p_context = nullptr;

    g->set_pin_mode(17, pin_mode::out);
    g->set_pin_mode(19, pin_mode::out);

    auto res = nrfx_uarte_init(&uart, &conf, [](nrfx_uarte_event_t const *p_event, void *p_context){
        sem.up();
    });

    uint8_t buf[] = { 'h', 'e', 'l', 'l', 'o', '\n' };
    res = nrfx_uarte_tx(&uart, buf, 6);

    tos::launch(led1_task);
    tos::launch(led2_task);
}