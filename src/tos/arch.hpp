//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stddef.h>

extern "C"
{
void* tos_stack_alloc(size_t size);
void tos_stack_free(void*);
void tos_enable_interrupts();
void tos_disable_interrupts();
}
