//
// Created by Mehmet Fatih BAKIR on 25/09/2018.
//

#include <tos/compiler.hpp>
#include <tos/debug.hpp>

[[noreturn]] void tos_force_get_failed(void*)
{
    //TODO: die
    tos::kern::fatal("force_get failed!");
}