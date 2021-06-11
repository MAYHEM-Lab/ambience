#include "irq.h"
#include "nvic_common.h"
#include <boost/preprocessor.hpp>

extern "C" {
struct [[gnu::packed]] nvic_vector {
    using func_ptr = void (*)();
    nvic_common common = nvic_common::default_handlers();
    func_ptr ptrs[90];
};

extern constexpr nvic_vector g_pfnVectors{.common = nvic_common::default_handlers(),
                                          .ptrs = {
#define IRQ(x) x,
#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_NAME, _irq.h))
#undef IRQ
                                          }};
}