#pragma once

/**
 * This header is used to include the correct CMSIS device header.
 */

#if defined(TOS_PLATFORM_nrf52)
#include <nrf.h>
#elif defined(TOS_PLATFORM_cc32xx)

#endif