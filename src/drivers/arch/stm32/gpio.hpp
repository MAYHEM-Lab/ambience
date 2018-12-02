#include <drivers/common/gpio.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <cstdint>
#include <array>

namespace tos
{
	namespace stm32
	{
		struct port_def {
			uintptr_t which;
			rcc_periph_clken rcc;
		};

		struct pin_t
		{
			const port_def* port;
			uint16_t pin;
		};

		constexpr inline std::array<port_def, 8> ports = {
			port_def {
				GPIOA,
				RCC_GPIOA
			}, port_def {
				GPIOB,
				RCC_GPIOB
			}, port_def {
				GPIOC,
				RCC_GPIOC
			}, port_def {
				GPIOD,
				RCC_GPIOD
			}
		};

		class gpio
		{
		public:
			using pin_type = pin_t;
			
			/**
			 * Sets the given pin to be an output
			 */
			void set_pin_mode(const pin_type& pin, pin_mode::output_t)
			{
			    rcc_periph_clock_enable(pin.port->rcc);
				gpio_set_mode(pin.port->which, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin.pin);
			}

			void set_pin_mode(const pin_type& pin, pin_mode::input_t)
            {
			    rcc_periph_clock_enable(pin.port->rcc);
			    gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, pin.pin);
            }

            void set_pin_mode(const pin_type& pin, pin_mode::in_pullup_t)
            {
                rcc_periph_clock_enable(pin.port->rcc);
                gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, pin.pin);
                write(pin, digital::high);
            }

            void set_pin_mode(const pin_type& pin, pin_mode::in_pulldown_t)
            {
                rcc_periph_clock_enable(pin.port->rcc);
                gpio_set_mode(pin.port->which, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, pin.pin);
                write(pin, digital::low);
            }

			void write(const pin_type& pin, digital::high_t)
            {
			    gpio_set(pin.port->which, pin.pin);
            }

            void write(const pin_type& pin, digital::low_t)
            {
                gpio_clear(pin.port->which, pin.pin);
            }

		private:
		};
	}

	namespace tos_literals
	{
		constexpr stm32::pin_t operator""_pin(unsigned long long pin)
		{
			auto port_index = pin / 16;
			return { &stm32::ports[port_index], 1 << (pin % 16) };
		}
	}

	stm32::gpio open_impl(tos::devs::gpio_t)
    {
	    return {};
    }
}