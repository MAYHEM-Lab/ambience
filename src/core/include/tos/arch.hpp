//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stddef.h>
#include <tos_arch.hpp>
#include <tos/compiler.hpp>

extern "C"
{
[[noreturn]] void tos_force_reset();

/**
 * Unconditionally enables all interrupts
 * This function should not be used directly.
 *
 * Please use `tos::enable_interrupts()` that supports
 * nested disabling of interrupts.
 */
void tos_enable_interrupts();

/**
 * Unconditionally disables all interrupts
 * This function should not be used directly.
 *
 * Please use `tos::disable_interrupts()` that supports
 * nested disabling of interrupts.
 */
void tos_disable_interrupts();
}
