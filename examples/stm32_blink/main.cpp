//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <tos/semaphore.hpp>

#include <drivers/arch/stm32/drivers.hpp>
#include <util/include/tos/fixed_fifo.hpp>
#include <util/include/tos/mem_stream.hpp>
#include <tos/print.hpp>

#include <drivers/arch/stm32/timer.hpp>

tos::semaphore set{1}, clear{0};

void usart_setup()
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, GPIO_USART2_RX);

    usart_set_baudrate(USART2, 9600);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    nvic_enable_irq(NVIC_USART2_IRQ);
    usart_enable_rx_interrupt(USART2);
    usart_enable(USART2);
}

tos::fixed_fifo<uint8_t, 32, tos::ring_buf> rx_buf;
const uint8_t* tx_it;
const uint8_t* tx_end;
tos::semaphore rx_s{0};
tos::semaphore tx_s{0};

void usart2_isr()
{
    if ((USART_CR1(USART2) & USART_CR1_RXNEIE) &&
        (USART_SR(USART2) & USART_SR_RXNE)) {
        rx_buf.push(usart_recv(USART2));
        rx_s.up_isr();
    }

    if ((USART_CR1(USART2) & USART_CR1_TXEIE) &&
             (USART_SR(USART2) & USART_SR_TXE))
    {
        usart_send(USART2, *tx_it++);
        if (tx_it == tx_end)
        {
            usart_disable_tx_interrupt(USART2);
            tx_s.up_isr();
        }
    }
}

void usart_write(tos::span<const uint8_t> buf)
{
    tx_it = buf.begin();
    tx_end = buf.end();
    usart_enable_tx_interrupt(USART2);
    tx_s.down();
}

void usart_write(tos::span<const char> buf)
{
    usart_write(tos::span<const uint8_t>{ (const uint8_t*)buf.data(), buf.size() });
}

void blink_task(void*)
{
	using namespace tos::tos_literals;

	auto g = tos::open(tos::devs::gpio);

    usart_setup();

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

	g.set_pin_mode(5_pin, tos::pin_mode::out);

	char buf[20];
	tos::omemory_stream oms{buf};
	tos::println(oms, int(rcc_apb1_frequency));
	usart_write(oms.get());

    while (true)
    {
        set.down();
        g.write(5_pin, tos::digital::high);
        while (true)
        {
            using namespace std::chrono_literals;
            alarm.sleep_for(1s);
            usart_write("l");
            g.write(5_pin, tos::digital::low);

            alarm.sleep_for(1s);
            usart_write("h");
            g.write(5_pin, tos::digital::high);
        }
        rx_s.down();
        uint8_t buf[] = { 'h', 'i', rx_buf.pop(), '\n' };
        usart_write(buf);

        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }

        clear.up();
    }
}

void off_task(void*)
{
    while (true)
    {
        clear.down();
        gpio_clear(GPIOA, GPIO5);
        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }
        set.up();
    }
}

void tos_main()
{
    tos::launch(blink_task);
    tos::launch(off_task);
}