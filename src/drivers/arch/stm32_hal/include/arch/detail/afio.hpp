#pragma once

#if defined(STM32L4)
#include "afio/afio_l4.hpp"
#elif defined(STM32L0)
#elif defined(STM32F1)
#elif defined(STM32F7)
#include "afio/afio_f7.hpp"
#endif