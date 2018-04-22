//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <drivers/arch/avr/usart.hpp>

extern "C"
{
void tos_set_stack_ptr(void* ptr);
void tos_power_down();
void* tos_stack_alloc(size_t size);
void tos_stack_free(void*);
void tos_shutdown();
void tos_enable_interrupts();
void tos_disable_interrupts();
}

namespace tos
{
	//TODO: count interrupt disables
	namespace detail
	{
		extern int8_t disable_depth;
	}

    inline void enable_interrupts() 
    {
    	detail::disable_depth--;
    	if (detail::disable_depth == 0)
    	{
			//tos::avr::write_sync("enable\n", 8);
    		tos_enable_interrupts();
    	} 
    }

    inline void disable_interrupts() 
    {
    	if (detail::disable_depth == 0)
    	{
			//tos::avr::write_sync("disable\n", 9);
    		tos_disable_interrupts();
    	}
    	detail::disable_depth++;
    }

    struct int_guard
    {
    public:
    	int_guard()
    	{
    		disable_interrupts();    			
    	}
    	~int_guard()
    	{
    		enable_interrupts();
    	}

    	int_guard(int_guard&&) = delete;
    };
}