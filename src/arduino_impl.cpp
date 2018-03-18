#include <avr/io.h>
#include <LowPower.h>
#include <Arduino.h>

#include <util/atomic.h>
#include "tos/atomic.hpp"
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"

namespace ft
{
	namespace impl
	{
		void set_stack_ptr(void* ptr)
		{
		  SP = ptr;
		}

		void power_down()
		{
		  Serial.flush();
		  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
		}
	}
}

namespace tos
{
	template <class T>
	void atomic<T>::add(const T& t)
	{
	    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	    {
	      m_t += t;
	    }
	}

	template class atomic<char>;
	template class atomic<short>;
	template class atomic<int>;
	template class atomic<unsigned int>;
	template class atomic<long>;
}

// the optional arduino yield thing
extern "C" void yield()
{
	if (!ft::self())
	{
		return;
	}
	ft::this_thread::yield();
}
