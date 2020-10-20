#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/periph/stm32l4_rtc.hpp>

using bs = tos::bsp::board_spec;

void rtc_main() {
    {
        auto usart = bs::default_com::open();
        tos::debug::serial_sink sink{&usart};
        tos::debug::detail::any_logger log_{&sink};
        log_.set_log_level(tos::debug::log_level::all);
        tos::debug::set_default_log(&log_);

        LOG("Hello");
    }

    auto rtc_res = tos::stm32::l4::rtc::open();
    if (auto setup_res = rtc_res; !setup_res) {
        LOG_ERROR("Failed:", int(force_error(setup_res)));
    }

    auto& rtc = force_get(rtc_res);
    using namespace std::chrono_literals;
    rtc.set_wakeup_timer(5s);

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnterSTANDBYMode();

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, rtc_main);
}