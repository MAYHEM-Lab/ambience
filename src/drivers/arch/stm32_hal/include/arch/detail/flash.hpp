#pragma once

#include "flash_common.hpp"
#if defined(STM32L4)
#include "flash_l4.hpp"
#elif defined(STM32L0)
#include "flash_l0.hpp"
#elif defined(STM32F1)
#include "flash_f1.hpp"
#elif defined(STM32F7)
#include "flash/flash_f7.hpp"
#endif