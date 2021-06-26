#include "irq.h"
#include <boost/preprocessor.hpp>
#include <cstddef>
#include <tos/arm/startup_common.hpp>

extern "C" {
[[gnu::section(".isr_vector"), gnu::used]] extern constexpr tos::arm::nvic_vector<
    static_cast<size_t>(tos::arm::external_interrupts::size)>
    g_pfnVectors{.common = tos::arm::vector_table::default_table(),
                 .ptrs = {
#define IRQ(x) &x,
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
                 }};
}