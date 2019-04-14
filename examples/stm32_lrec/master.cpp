//
// Created by fatih on 1/3/19.
//

#include <tos/ft.hpp>
#include <arch/stm32/drivers.hpp>
#include <tos/print.hpp>
#include <common/dht22.hpp>
#include <common/xbee.hpp>
#include <tos/mem_stream.hpp>
#include "app.hpp"
#include <common/ina219.hpp>
#include <libopencm3/stm32/iwdg.h>
#include <common/bme280.hpp>

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/itm.h>
#include <libopencm3/stm32/rtc.h>

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

auto setup_usart1 = [](auto& g){
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
};

auto disable_usart1 = [](auto& g){
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;
    g.set_pin_mode(rx_pin, tos::pin_mode::in_pullup);
    g.set_pin_mode(tx_pin, tos::pin_mode::in_pullup);
};

auto setup_usart2 = [](auto& g){
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_USART3_REMAP_PARTIAL_REMAP;

    auto tx_pin = 42_pin;
    auto rx_pin = 43_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
    gpio_set_mode(GPIO_BANK_USART3_PR_TX, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_PR_TX);
};

auto setup_usart0 = [](auto& g){
    using namespace tos::tos_literals;

    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
};

enum class slave_errors
{
    non_responsive,
    garbage,
    no_checksum,
    checksum_error
};

template <class GpioT, class UartT, class AlarmT>
tos::expected<temp::sample, slave_errors>  read_slave(GpioT& g, UartT& usart, AlarmT& alarm)
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto mosfet_pin = 7_pin;

    // wake up the slave and wait for it to boot
    //usart.clear();
    g.write(mosfet_pin, tos::digital::high);
    alarm.sleep_for(1000ms);
    iwdg_reset();

    std::array<char, 2> wait;
    auto res = usart.read(wait, alarm, 5s);

    if (res.size() != 2 || res[0] != 'h' || res[1] != 'i')
    {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::non_responsive);
    }

    std::array<char, sizeof(temp::sample)> buf;

    tos::print(usart, "go");
    alarm.sleep_for(10s);
    iwdg_reset();
    auto r = usart.read(buf, alarm, 5s);

    if (r.size() != buf.size())
    {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::garbage);
    }

    std::array<char, 1> chk_buf;

    auto chkbuf = usart.read(chk_buf, alarm, 5s);
    if (chkbuf.size() == 0)
    {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::no_checksum);
    }
    iwdg_reset();

    g.write(mosfet_pin, tos::digital::low);

    alarm.sleep_for(5ms);

    uint8_t chk = 0;
    for (char c : buf)
    {
        chk += uint8_t (c);
    }

    if (uint8_t(chkbuf[0]) != chk)
    {
        return tos::unexpected(slave_errors::checksum_error);
    }

    temp::sample s;
    memcpy(&s, buf.data(), sizeof(temp::sample));
    return s;
}

tos::event rtc_ev;
void rtc_isr(void)
{
    rtc_clear_flag(RTC_SEC);

    rtc_ev.fire_isr();
}

auto xbee_task = [](auto& g, auto& log)
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;
    namespace xbee = tos::xbee;

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    setup_usart0(g);
    auto xbee_ser = tos::open(tos::devs::usart<0>, tos::uart::default_9600);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    auto dht_pin = 1_pin;
    g.set_pin_mode(dht_pin, tos::pin_mode::in_pullup);

    disable_usart1(g);
    g.set_pin_mode(7_pin, tos::pin_mode::out);
    g.write(7_pin, tos::digital::low);

    auto d = tos::make_dht(g, delay);

    g.set_pin_mode(11_pin, tos::pin_mode::out);
    g.write(11_pin, tos::digital::low);
    alarm.sleep_for(100ms); // let it die

    tos::stm32::twim t { 22_pin, 23_pin };

    tos::ina219<tos::stm32::twim&> ina{ tos::twi_addr_t{0x40}, t };
    using namespace tos::bme280;
    bme280 b{ {BME280_I2C_ADDR_PRIM}, &t, delay };
    b.set_config();

    auto rd_slv = [&g, &alarm] {
        iwdg_reset();

        setup_usart1(g);
        auto slave_uart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);
        gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);

        auto r = read_slave(g, slave_uart, alarm);
        iwdg_reset();

        disable_usart1(g);
        return r;
    };

    uint32_t lp = 0;
    while (true)
    {
        g.write(45_pin, tos::digital::low);

        iwdg_reset();

        tos::stm32::adc a { &tos::stm32::detail::adcs[0] };

        uint8_t x[1];
        x[0] = 16;
        a.set_channels(x);

        uint32_t cpu_temp = [&]{
            tos::int_guard ig;
            uint32_t total = 0;
            for (int i = 0; i < 100; ++i)
            {
                total += a.read();
            }
            return total / 100;
        }();

        constexpr auto v25 = 1700;
        constexpr auto slope = 2;

        auto read_c = [&]{
            float v = a.read();
            return (v25 - v) / slope + 25;
        };

        auto slv = rd_slv();

        int curr = ina.getCurrent_mA();
        int v = ina.getBusVoltage_V();
        iwdg_reset();

        int tries = 1;
        auto res = d.read(dht_pin);

        while (res != tos::dht_res::ok && tries < 5) {
            tos::println(log, "dht non-responsive");
            alarm.sleep_for(2s);
            res = d.read(dht_pin);
            ++tries;
            iwdg_reset();
        }

        if (tries == 5) {
            d.temperature = -1;
            d.humidity = -1;
        }

        std::array<temp::sample, 2> samples = {
                temp::sample{d.temperature, d.humidity, float(cpu_temp)},
                {}
        };

        if (slv)
        {
            samples[1] = force_get(slv);
        }

        b->enable();
        delay(70ms);
        auto bme_res = b->read();
        b->sleep();

        std::array<char, 80> buf;
        tos::msgpack::packer p{buf};
        auto arr = p.insert_arr(8 + (slv ? 4 : 0) + (bme_res ? 3 : 0));

        arr.insert(lp++);

        arr.insert(uint32_t(temp::master_id));
        arr.insert(samples[0].temp);
        arr.insert(samples[0].humid);
        arr.insert(samples[0].cpu);

        arr.insert(bool(slv));
        if (slv)
        {
            arr.insert(uint32_t(temp::slave_id));
            arr.insert(samples[1].temp);
            arr.insert(samples[1].humid);
            arr.insert(samples[1].cpu);
        }

        arr.insert(int32_t(curr));
        arr.insert(int32_t(v));

        if (bme_res)
        {
            auto [pres, temp, humid] = force_get(bme_res);

            arr.insert(pres);
            arr.insert(temp);
            arr.insert(humid);
        }

        g.write(11_pin, tos::digital::high);

        alarm.sleep_for(100ms); // let it wake up
        auto r = xbee::read_modem_status(xbee_ser, alarm);

        if (!r)
        {
            tos::println(log, "xbee non responsive");
        }

        constexpr xbee::addr_16 base_addr{0x0001};

        xbee::tx16_req req{base_addr, tos::raw_cast<const uint8_t>(p.get()), xbee::frame_id_t{0xAB}};
        iwdg_reset();
        xbee::write_to(xbee_ser, req);

        alarm.sleep_for(100ms);
        auto tx_r = xbee::read_tx_status(xbee_ser, alarm);

        if (tx_r) {
            auto &res = tos::force_get(tx_r);
            if (res.status == xbee::tx_status::statuses::success) {
                tos::println(log, "xbee sent!");
            } else {
                tos::println(log, "xbee send failed");
            }
        } else {
            tos::println(log, "xbee non responsive");
        }

        g.write(11_pin, tos::digital::low);

        tos::println(log, "sleeping...");
        iwdg_reset();
        //tos::this_thread::block_forever();

        auto slept = 0s;

        g.write(45_pin, tos::digital::high);
        nvic_enable_irq(NVIC_RTC_IRQ);
        rtc_interrupt_enable(RTC_SEC);

        while (slept < 5s)
        {
            rtc_ev.wait();
            iwdg_reset();
            slept += 1s;
        }

        rtc_interrupt_disable(RTC_SEC);
        nvic_disable_irq(NVIC_RTC_IRQ);
    }
};

static tos::stack_storage<1576> sstack __attribute__ ((section (".noinit")));
void master_task()
{
    rtc_auto_awake(RCC_LSE, 0x7fff);
    nvic_set_priority(NVIC_RTC_IRQ, 1);

    iwdg_set_period_ms(15'000);
    iwdg_start();
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    namespace xbee = tos::xbee;

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(45_pin, tos::pin_mode::out);

    for (int i = 0; i < 3; ++i)
    {
        g.write(45_pin, tos::digital::high);
        delay(500ms);
        g.write(45_pin, tos::digital::low);
        delay(500ms);
    }
    g.write(45_pin, tos::digital::high);

    struct x : tos::self_pointing<x> {
        int write(tos::span<const char> x) { return x.size(); }
    } l;

    tos::semaphore s{0};
    tos::launch(sstack, [&]{
        xbee_task(g, l);
       s.up();
    });
    s.down();
}

static tos::stack_storage<1024> stack;
void tos_main()
{
    tos::launch(stack, master_task);
}
