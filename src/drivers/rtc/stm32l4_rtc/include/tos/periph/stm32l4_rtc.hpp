#pragma once

#include <tos/expected.hpp>
#include <stm32_hal/rtc.hpp>
#include <chrono>

namespace tos::stm32::l4 {
enum class rtc_errors {
    clock_error,
    init_error
};
class rtc {
public:
    static expected<rtc, rtc_errors> open();

    // Sets up a timer able to wake up the processor from deep sleep by rebooting.
    // ms must be less than or equal to 32 seconds.
    void set_wakeup_timer(std::chrono::milliseconds ms);

private:

    RTC_HandleTypeDef m_rtc;
};
}