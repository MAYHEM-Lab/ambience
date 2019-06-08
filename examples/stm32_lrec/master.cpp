//
// Created by fatih on 1/3/19.
//

#include <tos/ft.hpp>
#include <arch/drivers.hpp>
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
    for (volatile int i = 0; i < end; ++i) {
        __asm__ __volatile__ ("nop");
    }
};

auto setup_usart1 = [](auto &g) {
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
};

auto disable_usart1 = [](auto &g) {
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;
    g.set_pin_mode(rx_pin, tos::pin_mode::in_pullup);
    g.set_pin_mode(tx_pin, tos::pin_mode::in_pullup);
};

auto setup_usart2 = [](auto &g) {
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_USART3_REMAP_PARTIAL_REMAP;

    auto tx_pin = 42_pin;
    auto rx_pin = 43_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
    gpio_set_mode(GPIO_BANK_USART3_PR_TX, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_PR_TX);
};

auto setup_usart0 = [](auto &g) {
    using namespace tos::tos_literals;

    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
};

auto disable_usart0 = [](auto &g) {
    using namespace tos::tos_literals;

    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in_pullup);
    g.set_pin_mode(tx_pin, tos::pin_mode::in_pullup);
};

enum class slave_errors {
    non_responsive,
    garbage,
    no_checksum,
    checksum_error
};

struct dht_data {
    float temp;
    float humid;
};

struct node_data {
    dht_data dht;
    float cpu;
};

template<class GpioT, class UartT, class AlarmT>
tos::expected<node_data, slave_errors> read_slave(GpioT &g, UartT &usart, AlarmT &alarm) {
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

    if (res.size() != 2 || res[0] != 'h' || res[1] != 'i') {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::non_responsive);
    }

    std::array<char, sizeof(node_data)> buf;

    tos::print(usart, "go");
    alarm.sleep_for(10s);
    iwdg_reset();
    auto r = usart.read(buf, alarm, 5s);

    if (r.size() != buf.size()) {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::garbage);
    }

    std::array<char, 1> chk_buf;

    auto chkbuf = usart.read(chk_buf, alarm, 5s);
    if (chkbuf.size() == 0) {
        g.write(mosfet_pin, tos::digital::low);
        return tos::unexpected(slave_errors::no_checksum);
    }
    iwdg_reset();

    g.write(mosfet_pin, tos::digital::low);

    alarm.sleep_for(5ms);

    uint8_t chk = 0;
    for (char c : buf) {
        chk += uint8_t(c);
    }

    if (uint8_t(chkbuf[0]) != chk) {
        return tos::unexpected(slave_errors::checksum_error);
    }

    node_data s;
    memcpy(&s, buf.data(), sizeof(node_data));
    return s;
}

tos::event rtc_ev;

void rtc_isr(void) {
    rtc_clear_flag(RTC_SEC);
    rtc_ev.fire_isr();
}

struct ina_data {
    int32_t voltage;
    int32_t current;
};

struct data {
    uint32_t seq;
    node_data master;
    std::optional<node_data> slave;
    std::optional<bme280_data> bme;
    ina_data ina;
};

void pack(tos::msgpack::packer &p, const data &d) {
    auto arr = p.insert_arr(8 + (d.slave ? 4 : 0) + (d.bme ? 3 : 0));
    arr.insert(d.seq);

    arr.insert(uint32_t(temp::master_id));
    arr.insert(d.master.dht.temp);
    arr.insert(d.master.dht.humid);
    arr.insert(d.master.cpu);

    arr.insert(bool(d.slave));
    if (d.slave) {
        arr.insert(uint32_t(temp::slave_id));
        arr.insert(d.slave->dht.temp);
        arr.insert(d.slave->dht.humid);
        arr.insert(d.slave->cpu);
    }

    arr.insert(int32_t(d.ina.current));
    arr.insert(int32_t(d.ina.voltage));

    if (d.bme) {
        auto[pres, temp, humid] = *d.bme;

        arr.insert(pres);
        arr.insert(temp);
        arr.insert(humid);
    }
}

void rtc_start()
{
    nvic_enable_irq(NVIC_RTC_IRQ);
    rtc_interrupt_enable(RTC_SEC);
}
void rtc_stop()
{
    rtc_interrupt_disable(RTC_SEC);
    nvic_disable_irq(NVIC_RTC_IRQ);
}

void rtc_sleep(std::chrono::seconds len) {
    using namespace std::chrono_literals;
    auto slept = 0s;

    while (slept < len) {
        rtc_ev.wait();
        iwdg_reset();
        slept += 1s;
    }
}

static tos::fixed_fifo<data, 40> backlog;
auto sense_task = [](auto &g, auto &alarm) {
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto dht_pin = 1_pin;
    g.set_pin_mode(dht_pin, tos::pin_mode::in_pullup);

    disable_usart1(g);
    g.set_pin_mode(7_pin, tos::pin_mode::out);
    g.write(7_pin, tos::digital::low);

    auto d = tos::make_dht(g, delay);

    tos::stm32::twim t{22_pin, 23_pin};

    tos::ina219<tos::stm32::twim &> ina{tos::twi_addr_t{0x40}, t};

    using namespace tos::bme280;
    bme280 b{{BME280_I2C_ADDR_PRIM}, &t, delay};
    b.set_config();

    rtc_start();

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
    while (true) {
        g.write(45_pin, tos::digital::low);
        iwdg_reset();

        tos::stm32::adc a{&tos::stm32::detail::adcs[0]};

        uint8_t x[1];
        x[0] = 16;
        a.set_channels(x);

        uint32_t cpu_temp = [&] {
            tos::int_guard ig;
            uint32_t total = 0;
            for (int i = 0; i < 100; ++i) {
                total += a.read();
            }
            return total / 100;
        }();

        auto slv = rd_slv();

        int tries = 1;
        auto res = d.read(dht_pin);

        while (res != tos::dht_res::ok && tries < 5) {
            alarm.sleep_for(2s);
            res = d.read(dht_pin);
            ++tries;
            iwdg_reset();
        }

        if (tries == 5) {
            d.temperature = -1;
            d.humidity = -1;
        }

        data sample;
        b->enable();
        alarm->sleep_for(70ms);
        sample.bme = std::optional<bme280_data>(b->read());
        b->sleep();

        sample.seq = lp++;
        sample.slave = std::optional<node_data>(slv);
        sample.master.dht.temp = d.temperature;
        sample.master.dht.humid = d.humidity;
        sample.master.cpu = cpu_temp;
        sample.ina.current = ina.getCurrent_mA();
        sample.ina.voltage = ina.getBusVoltage_V();

        backlog.push(std::move(sample));

        iwdg_reset();
        g.write(45_pin, tos::digital::high);
        rtc_sleep(4min + 50s);
    }
};

auto xbee_task = [](auto &g, auto &log, auto &alarm) {
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;
    namespace xbee = tos::xbee;

    setup_usart0(g);
    auto xbee_ser = tos::open(tos::devs::usart<0>, tos::uart::default_9600);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    g.set_pin_mode(11_pin, tos::pin_mode::out);
    g.write(11_pin, tos::digital::low);
    alarm.sleep_for(100ms); // let it die

    while (true) {
        auto sample = backlog.pop();

        std::array<char, 80> buf;
        tos::msgpack::packer p{buf};

        pack(p, sample);

        int retry = 0;
        bool sent = false;
        do {
            g.write(11_pin, tos::digital::high);

            alarm.sleep_for(100ms); // let it wake up
            auto r = xbee::read_modem_status(xbee_ser, alarm);

            if (!r) {
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
                    sent = true;
                } else {
                    tos::println(log, "xbee send failed");
                }
            } else {
                tos::println(log, "xbee non responsive");
            }

            g.write(11_pin, tos::digital::low);

            if (!sent) {
                if (retry % 3 == 2) {
                    rtc_sleep(4min + 30s);
                } else {
                    rtc_sleep(10s);
                }
            }
            ++retry;
        } while (!sent);
    }
};

static tos::stack_storage<1576> xbee_stack;
static tos::stack_storage<1576> sense_stack;

void master_task() {
    rtc_auto_awake(RCC_LSE, 0x7fff);
    nvic_set_priority(NVIC_RTC_IRQ, 1);

    iwdg_set_period_ms(15'000);
    iwdg_start();
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    namespace xbee = tos::xbee;

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(45_pin, tos::pin_mode::out);

    for (int i = 0; i < 3; ++i) {
        g.write(45_pin, tos::digital::high);
        delay(200ms);
        g.write(45_pin, tos::digital::low);
        delay(200ms);
    }
    g.write(45_pin, tos::digital::high);

    struct x : tos::self_pointing<x> {
        int write(tos::span<const char> x) { return x.size(); }
    } l;

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::launch(sense_stack, [&] {
        sense_task(g, alarm);
    });

    tos::launch(xbee_stack, [&] {
        xbee_task(g, l, alarm);
    });

    tos::this_thread::block_forever();
}

static tos::stack_storage<1024> stack;

void tos_main() {
    tos::launch(stack, master_task);
}
