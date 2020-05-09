#pragma once

#include <arch/cmsis.hpp>

namespace tos {
constexpr bool memmanage_exception_supported() {
#if defined(SCB_SHCSR_MEMFAULTENA_Msk)
    return true;
#endif
    return false;
}
}