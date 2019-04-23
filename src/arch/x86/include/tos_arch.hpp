//
// Created by fatih on 3/20/18.
//

#pragma once

#include <boost/asio/io_service.hpp>

extern "C"
{
void tos_set_stack_ptr(char* ptr) __attribute__((always_inline));
inline void tos_set_stack_ptr(char* ptr)
{
    __asm__ __volatile__("movq %0, %%rsp" : : "r"(ptr) : "memory");
}
}

boost::asio::io_service& get_io();