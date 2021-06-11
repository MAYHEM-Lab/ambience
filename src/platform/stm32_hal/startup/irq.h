#pragma once

#include <boost/preprocessor.hpp>

extern "C" {
#define IRQ(x) void x();
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
}

namespace tos::stm32 {
enum class external_interrupts
{
#define IRQ(x) x,
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
    size
};
} // namespace tos::stm32
