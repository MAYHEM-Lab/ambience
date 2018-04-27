//
// Created by fatih on 4/26/18.
//

#pragma once

extern "C"
{
/*#define tos_set_stack_ptr(x) \
{ \
 __asm__ __volatile__("movi a0, 0;" ::: "memory"); \
 __asm__ __volatile__("mov sp,%0;" :: "r" ((x) - 16) : "memory"); \
}*/

void tos_set_stack_ptr(void*);

inline void* read_sp()
{
   void* sp;
   __asm__ __volatile__("mov %0, a1;" : "=r"(sp) ::);
   return sp;
}
}