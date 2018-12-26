//
// Created by Mehmet Fatih BAKIR on 11/10/2018.
//

#include <arch/avr/drivers.hpp>
#include <common/xbee.hpp>
#include <common/alarm.hpp>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

#include <array>
#include <avr/wdt.h>
#include <common/dht22.hpp>
#include <util/delay.h>
#include <tos/mem_stream.hpp>
#include "app.hpp"
#include <common/ina219.hpp>
#include <tos/semaphore.hpp>

char trace_buf[128];
tos::omemory_stream trace{ trace_buf };

namespace temp
{
    template <class GpioT, class AlarmT, class UsartT>
    sample read_slave(GpioT& gpio, AlarmT& alarm, UsartT& usart)
    {
        using namespace tos::tos_literals;
        using namespace std::chrono_literals;

		// wake up the slave and wait for it to boot
		//usart.clear();
        gpio.write(11_pin, tos::digital::high);
        alarm.sleep_for(5000ms);

        std::array<char, 2> wait;
        auto res = usart.read(wait, alarm, 5s);

        if (res.size() != 2 || res[0] != 'h' || res[1] != 'i')
        {
            gpio.write(11_pin, tos::digital::low);
			tos::println(trace, "slave non-responsive", res.size(), res);
            return temp::sample{ -1, -1, -1 };
        }

        std::array<char, sizeof(temp::sample)> buf;

        tos::print(usart, "go");
        alarm.sleep_for(10s);
        auto r = usart.read(buf, alarm, 5s);

        if (r.size() != buf.size())
        {
            gpio.write(11_pin, tos::digital::low);
			tos::println(trace, "slave sent garbage");
            return temp::sample{ -1, -1, -1 };
        }

        std::array<char, 1> chk_buf;

        auto chkbuf = usart.read(chk_buf, alarm, 5s);
        if (chkbuf.size() == 0)
        {
            gpio.write(11_pin, tos::digital::low);
			tos::println(trace, "slave omitted checksum");
            return temp::sample{ -1, -1, -1 };
        }

        gpio.write(11_pin, tos::digital::low);
        alarm.sleep_for(5ms);

        uint8_t chk = 0;
        for (char c : buf)
        {
            chk += uint8_t (c);
        }

        usart.write(buf);
        usart.write(chkbuf);

        usart.write(tos::raw_cast<char>(tos::span<uint8_t>{&chk,1}));

        if (uint8_t(chkbuf[0]) != chk)
        {
			tos::println(trace, "slave checksum mismatch");
            return temp::sample{ -1, -1, -1 };
        }

		tos::println(trace, "slave success");
        temp::sample s;
        memcpy(&s, buf.data(), sizeof(temp::sample));
        return s;
    }
} // namespace temp

/**
 * This function takes the system to a deep sleep.
 *
 * The actual sleep duration could be longer than the requested time.
 *
 * System won't sleep if there are other runnable threads in the system.
 *
 * This function sleeps with the multiples of 8 seconds
 *
 * @param dur seconds to at least sleep for
 */
void hibernate(tos::avr::wdt& wdt, std::chrono::seconds dur)
{
    using namespace std::chrono_literals;

    wdt_reset();
    while (dur > 8s)
    {
        wdt.wait(tos::avr::wdt_times::to_8s);
        dur -= 8s;

        //wdt_enable(WDTO_8S);
        //WDTCSR |= (1 << WDIE);
        //wait_wdt();
    }
}

void reset_cpu();

void tx_task()
{
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);
    tos::println(usart, "alive");

    auto gpio = tos::open(tos::devs::gpio);

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(11_pin, tos::pin_mode::out);
    gpio.set_pin_mode(12_pin, tos::pin_mode::in_pullup);
    gpio.write(7_pin, tos::digital::low);
    gpio.write(11_pin, tos::digital::low);

    while (true) {
        using namespace std::chrono_literals;

        // reset the trace stream
        trace = tos::omemory_stream{trace_buf};

        {
            tos::avr::wdt_resetter res;
            res.start(50s);

            {
                auto tmr = tos::open(tos::devs::timer<1>);
                auto alarm = tos::open(tos::devs::alarm, *tmr);

                using namespace std::chrono_literals;
                alarm.sleep_for(2s);

                gpio.set_pin_mode(12_pin, tos::pin_mode::in_pullup);

                auto d = tos::make_dht(gpio, [](std::chrono::microseconds us) {
                    _delay_us(us.count());
                });

                int tries = 1;
                auto res = d.read(12_pin);

                while (res != tos::dht_res::ok && tries < 5) {
                    tos::println(trace, "dht non-responsive");
                    alarm.sleep_for(2s);
                    res = d.read(12_pin);
                    ++tries;
                }

                if (tries == 5) {
                    d.temperature = -1;
                    d.humidity = -1;
                }

                std::array<temp::sample, 2> samples = {
                        temp::sample{d.temperature, d.humidity, temp::GetTemp(alarm)},
                        temp::read_slave(gpio, alarm, usart)
                };

                tos::println(trace, int(samples[0].temp), int(samples[0].cpu));
                tos::println(trace, int(samples[1].cpu));

                namespace xbee = tos::xbee;
                gpio.write(7_pin, tos::digital::high);

                alarm.sleep_for(100ms);
                auto r = xbee::read_modem_status(usart, alarm);

                if (r) {
                    tos::xbee_s1<tos::avr::usart0> x{usart};

                    static char msg_buf[100];
                    tos::omemory_stream buff{msg_buf};

                    temp::print(buff, temp::master_id, samples[0]);
                    temp::print(buff, temp::slave_id, samples[1]);

                    constexpr xbee::addr_16 base_addr{0x0010};
                    xbee::tx16_req req{base_addr, tos::raw_cast<const uint8_t>(buff.get()), xbee::frame_id_t{1}};
                    x.transmit(req);

                    alarm.sleep_for(100ms);
                    auto tx_r = xbee::read_tx_status(usart, alarm);

                    if (tx_r) {
                        auto &res = force_get(tx_r);
                        if (res.status == xbee::tx_status::statuses::success) {
                            tos::println(trace, "xbee sent!");
                        } else {
                            tos::println(trace, "xbee send failed");
                        }
                    } else {
                        tos::println(trace, "xbee non responsive");
                    }
                } else {
                    tos::println(trace, "xbee non responsive");
                }

                gpio.write(7_pin, tos::digital::low);
            }

            tos::println(usart, trace.get());
        }

        tos::println(usart, "hibernating");

        tos::avr::wdt wdt;
        hibernate(wdt, 4min + 30s);
    }
}

void tos_main()
{
    static char sstack[512];
    tos::launch(tos::span<char>(sstack), tx_task);
}