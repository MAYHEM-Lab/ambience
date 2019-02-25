//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <arch/stm32/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

void blink_task()
{
	using namespace tos::tos_literals;

	auto g = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<3>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

	g.set_pin_mode(1_pin, tos::pin_mode::out);

    while (true)
    {
        using namespace std::chrono_literals;
        g.write(1_pin, tos::digital::high);
        alarm.sleep_for(1s);

        g.write(1_pin, tos::digital::low);
        alarm.sleep_for(1s);
    }
}

void tos_main()
{
    tos::launch(blink_task);
}
