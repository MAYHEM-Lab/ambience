//
// Created by fatih on 3/20/18.
//

#pragma once

#include <boost/asio/io_service.hpp>

extern "C"
{
inline void __attribute__((always_inline)) tos_set_stack_ptr(char* ptr)
{
    __asm__ __volatile__("movq %0, %%rsp" : : "r"(ptr) : "memory");
}

inline void* __attribute__((always_inline)) tos_get_stack_ptr()
{
    void* sp;
    __asm__ __volatile__("movq %%rsp, %0" : "=r"(sp) : : "memory");
    return sp;
}
}

boost::asio::io_service& get_io();