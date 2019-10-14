//
// Created by Mehmet Fatih BAKIR on 25/09/2018.
//

#include <tos/compiler.hpp>
#include <tos/debug/panic.hpp>

[[noreturn]] void tos_force_get_failed(void*)
{
    //TODO: die
    tos::debug::panic("force_get failed!");
}