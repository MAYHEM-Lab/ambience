//
// Created by fatih on 3/9/19.
//

#include <arch/x86/drivers.hpp>

#include <tos/ft.hpp>
#include <tos/devices.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>
#include <tos/streams.hpp>


#include "common.hpp"
#include "apps.hpp"

static void x86_main()
{
    auto tty = tos::open(tos::devs::tty<0>);
    tos::println(tty, "hello");
}

void tos_main()
{
    tos::launch(tos::def_stack, x86_main);
}