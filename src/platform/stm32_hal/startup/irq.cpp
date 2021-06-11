#include <boost/preprocessor.hpp>

extern "C" {
void Default_Handler() {
    while (true) {
    }
}

#define IRQ(x) [[gnu::weak, gnu::alias("Default_Handler")]] void x();
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
}