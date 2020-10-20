#include <stm32_hal/rcc.hpp>
#include <stm32_hal/rcc_ex.hpp>
#include <stm32l4xx_hal_pwr.h>
#include <tos/debug/log.hpp>
#include <tos/periph/stm32l4_rtc.hpp>

namespace tos::stm32::l4 {
expected<rtc, rtc_errors> rtc::open() {
    rtc res;

    RCC_PeriphCLKInitTypeDef PeriphClkInit = {};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        return unexpected(rtc_errors::clock_error);
    }

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    res.m_rtc = {};

    res.m_rtc.Instance = RTC;

    auto& init = res.m_rtc.Init;

    init.HourFormat = RTC_HOURFORMAT_24;
    init.AsynchPrediv = 127;
    init.SynchPrediv = 255;
    init.OutPut = RTC_OUTPUT_DISABLE;
    init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (auto init_res = HAL_RTC_Init(&res.m_rtc); init_res != HAL_OK) {
        return unexpected(rtc_errors::init_error);
    }

    if (HAL_RTCEx_SetLowPowerCalib(&res.m_rtc, RTC_LPCAL_SET) != HAL_OK)
    {
        return unexpected(rtc_errors::calib_error);
    }

    return res;
}

void rtc::set_wakeup_timer(std::chrono::milliseconds ms) {
#if defined(STM32L412xx)
    auto res = HAL_RTCEx_SetWakeUpTimer_IT(
        &m_rtc, ms.count() * 2, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0);
#elif
    auto res =
        HAL_RTCEx_SetWakeUpTimer_IT(&m_rtc, ms.count() * 2, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
#endif
    if (res != HAL_OK) {
        LOG_ERROR("Failed:", res);
    }
}
} // namespace tos::stm32::l4