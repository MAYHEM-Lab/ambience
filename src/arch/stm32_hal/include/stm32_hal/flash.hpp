/**
 * This file includes the correct stm32xxxx_flash header
 * depending on the tos arch configuration.
 */

#pragma once

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#include BOOST_PP_STRINGIZE(BOOST_PP_CAT(STM32_HAL_NAME, _hal_flash.h))
