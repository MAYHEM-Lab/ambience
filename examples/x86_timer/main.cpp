//
// Created by fatih on 12/10/18.
//

#include <tos/ft.hpp>
#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <iostream>

void tos_main()
{
    tos::launch(tos::alloc_stack, []{
         tos::x86::timer tmr;
         auto alarm = tos::open(tos::devs::alarm, tmr);

         while (true)
         {
             std::cout << "hi" << '\n';
             using namespace std::chrono_literals;
             alarm.sleep_for(1s);
         }
    });
}