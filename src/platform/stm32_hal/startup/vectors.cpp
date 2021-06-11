#include "irq.h"
#include "nvic_common.h"
#include <boost/preprocessor.hpp>
#include <cstddef>

extern "C" {
struct [[gnu::packed]] nvic_vector {
    using func_ptr = void (*)();
    nvic_common common = nvic_common::default_handlers();
    func_ptr ptrs[static_cast<size_t>(tos::stm32::external_interrupts::size)];
};

[[gnu::section(".isr_vector"), gnu::used]]
extern constexpr nvic_vector g_pfnVectors{.common = nvic_common::default_handlers(),
                                          .ptrs = {
#define IRQ(x) &x,
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
                                          }};
}