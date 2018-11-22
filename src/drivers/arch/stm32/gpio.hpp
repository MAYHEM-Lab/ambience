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
			}
		};

		class gpio
		{
		public:
			using pin_type = pin_t;

			explicit gpio(const port_def& def) {
				rcc_periph_clock_enable(def.rcc);
			}
			
			/**
			 * Sets the given pin to be an output
			 */
			void set_pin_mode(pin_type pin, pin_mode::output_t)
			{
				gpio_set_mode(pin.port->which, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin.pin);
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
}