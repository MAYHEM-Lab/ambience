#include <drivers/common/gpio.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <cstdint>

namespace tos
{
	namespace stm32
	{
		struct gpio_def {
			uintptr_t which;
			rcc_periph_clken rcc;
		};

		namespace gpios
		{
			constexpr inline gpio_def C {
				GPIOC,
				RCC_GPIOC
			};
		};

		class gpio
		{
		public:
			using pin_type = uint16_t;

			explicit gpio(const gpio_def& def) : m_def{ &def } {
				rcc_periph_clock_enable(m_def->rcc);
			}
			
			/**
			 * Sets the given pin to be an output
			 */
			void set_pin_mode(pin_type pin, pin_mode::output_t)
			{
				gpio_set_mode(m_def->which, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, pin);
			}

		private:

			const gpio_def* m_def;
		};
	}
}