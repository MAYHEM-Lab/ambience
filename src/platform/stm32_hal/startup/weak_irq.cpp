#include <boost/preprocessor.hpp>
#include "irq.h"

extern "C" {
void Default_Handler() {
    while (true) {
    }
}

#define IRQ(x) void x() __attribute__((weak, alias("Default_Handler")));
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
}