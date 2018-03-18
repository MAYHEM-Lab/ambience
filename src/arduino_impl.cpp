#include <avr/io.h>
#ifdef ARDUINO_CMAKE
#include <LowPower/LowPower.h>
#else
#include <LowPower.h>
#endif
#include <Arduino.h>

#include <util/atomic.h>
#include "tos/atomic.hpp"
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"

#include <string.h>

namespace ft
{
	namespace impl
	{
		void set_stack_ptr(void* ptr)
		{
			// return address is in the stack
			// if we just change the stack pointer
			// we can't return to the caller
			// copying the last 10 bytes from the original
			// stack to this stack so that we'll be able
			// to return
			memcpy(ptr - 10, (void*)SP, 10);
			SP = reinterpret_cast<uint16_t>(ptr - 10);
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
