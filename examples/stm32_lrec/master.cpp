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
#include <unwind.h>

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

    // need a proper API for this alternate function IO business ...
    //rcc_periph_clock_enable(RCC_AFIO);
    //AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;

    tos::stm32::twim t { 22_pin, 23_pin };

    tos::ina219<tos::stm32::twim&> ina{ tos::twi_addr_t{0x40}, t };

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

    tos::stm32::adc a { &tos::stm32::detail::adcs[0] };

    uint8_t x[1];
    x[0] = 16;
    a.set_channels(x);

    constexpr auto v25 = 1700;
    constexpr auto slope = 2;

    auto read_c = [&]{
        float v = a.read();
        return (v25 - v) / slope + 25;
    };

    uint32_t lp = 0;
    while (true)
    {
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
                temp::sample{d.temperature, d.humidity, read_c()},
                {}
        };

        if (slv)
        {
            samples[1] = force_get(slv);
        }

        samples[0].temp = temp::to_fahrenheits(samples[0].temp);
        samples[0].cpu = temp::to_fahrenheits(samples[0].cpu);

        samples[1].temp = temp::to_fahrenheits(samples[1].temp);
        samples[1].cpu = temp::to_fahrenheits(samples[1].cpu);

        std::array<char, 60> buf;
        tos::msgpack::packer p{buf};
        auto arr = p.insert_arr(10);

        arr.insert(lp++);

        arr.insert(samples[0].temp);
        arr.insert(samples[0].humid);
        arr.insert(samples[0].cpu);

        arr.insert(samples[1].temp);
        arr.insert(samples[1].humid);
        arr.insert(samples[1].cpu);

        arr.insert(int32_t(curr));
        arr.insert(int32_t(v));
        arr.insert(bool(slv));

        g.write(11_pin, tos::digital::high);

        alarm.sleep_for(100ms); // let it wake up
        auto r = xbee::read_modem_status(xbee_ser, alarm);

        if (!r)
        {
            tos::println(log, "xbee non responsive");
        }

        constexpr xbee::addr_16 base_addr{0x0010};

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
        alarm.sleep_for(5s);
        iwdg_reset();
    }
};

static char buf[1024];
static tos::stack_storage<1024> sstack __attribute__ ((section (".noinit")));
void master_task()
{
    iwdg_set_period_ms(15'000);
    iwdg_start();
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    namespace xbee = tos::xbee;

    auto g = tos::open(tos::devs::gpio);

    struct x : tos::self_pointing<x> {
        int write(tos::span<const char> x) { return x.size(); }
    } l;

    if (sstack.m_storage.__data[0] == 0x6B)
    {
        // something ran
        std::copy(std::begin(sstack.m_storage.__data), std::end(sstack.m_storage.__data), buf);
    }

    sstack.m_storage.__data[0] = 0x6B;

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
