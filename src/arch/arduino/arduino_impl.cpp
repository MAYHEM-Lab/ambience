#include <avr/io.h>
#include <avr/wdt.h>
#ifdef ARDUINO_CMAKE
#include <LowPower/LowPower.h>
#else
#include <LowPower.h>
#endif
#include <Arduino.h>

#undef putc

#include <util/atomic.h>
#include "tos/atomic.hpp"
#include "ft/include/tos/ft.hpp"
#include "ft/include/tos/tcb.hpp"
#include <tos/char_stream.hpp>

#include <tos_arch.hpp>

#include <string.h>

extern "C"
{
    #define soft_reset()        \
    do                          \
    {                           \
        wdt_enable(WDTO_15MS);  \
        for(;;)                 \
        {                       \
        }                       \
    } while(0)
    // Function Pototype
    void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

    // Function Implementation
    void wdt_init(void)
    {
        MCUSR = 0;
        wdt_disable();

        return;
    }

	void tos_set_stack_ptr(void* ptr) __attribute__((naked));
    void tos_set_stack_ptr(void* ptr)
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

	void tos_power_down()
	{
		Serial.flush();
		LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
	}

	void* tos_stack_alloc(size_t size)
	{
		return new char[size];
	}

	void tos_stack_free(void* data)
	{
		delete[] (char*)data;
	}

    void tos_shutdown()
    {
        soft_reset();
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

class serial_adapter
		: public tos::char_ostream
{
public:
	explicit serial_adapter(unsigned long baud)
	{
		Serial.begin(baud);
	}
	int write(const char* buf, size_t sz) override
	{
		return Serial.write(buf, sz);
	}
	void putc(char c) override
	{
		Serial.write(c);
	}
};

namespace tos
{
	namespace arch
	{
		char_ostream* debug_stream()
		{
			static serial_adapter ser(9600);
			return &ser;
		}
	}
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
