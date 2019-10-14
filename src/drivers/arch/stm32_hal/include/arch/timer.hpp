//
// Created by fatih on 11/30/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/timer.hpp>
#include <cstdint>
#include <iterator>
#include <memory>
#include <stm32_hal/tim.hpp>
#include <tos/function_ref.hpp>
#include <tos/scheduler.hpp>
#include <tos/track_ptr.hpp>

namespace tos::stm32 {
namespace detail {
/**
 * This struct is a literal type that just collects
 * relevant data about a specific general purpose timer.
 *
 * DO NOT instantiate objects of this type!
 */
struct gen_tim_def {
    TIM_TypeDef* tim;
    IRQn_Type irq;
    void (*rcc_en)();
};

inline const gen_tim_def gen_timers[] = {
    {TIM2, TIM2_IRQn, [] { __HAL_RCC_TIM2_CLK_ENABLE(); }},
#ifdef TIM3
    {TIM3, TIM3_IRQn, [] { __HAL_RCC_TIM3_CLK_ENABLE(); }}
#endif
};

} // namespace detail

/**
 * STM32s have a bunch of timers. Some of these timers are advanced,
 * and some of them are called general purpose.
 *
 * This class implements the Tos timer interface using the 1st channel
 * of a given general purpose timer.
 */
class general_timer
    : public self_pointing<general_timer>
    , public tracked_driver<general_timer, std::size(detail::gen_timers)> {
public:
    explicit general_timer(const detail::gen_tim_def& def);

    /**
     * Sets the frequency of the tick callback.
     */
    void set_frequency(uint16_t hertz);

    /**
     * Sets the function to be called on every tick of the timer.
     * @param fun function to be called on every tick.
     */
    void set_callback(tos::function_ref<void()> fun) { m_fun = fun; }

    /**
     * Gets the current value of the counter of the timer.
     * Together with get_period, these functions can be used to implement a clock.
     * @return the current value of the counter.
     */
    uint32_t get_counter() const;

    /**
     * Gets the period of the timer, i.e. the value of the counter at which the
     * tick callback is executed.
     * @return the period of the timer.
     */
    uint32_t get_period() const;

    void enable();
    void disable();

    auto native_handle() { return &m_handle; }

    void run_callback() { m_fun(); }

private:
    TIM_HandleTypeDef m_handle;
    const detail::gen_tim_def* m_def;
    tos::function_ref<void()> m_fun;
};
} // namespace tos::stm32

namespace tos {
inline stm32::general_timer open_impl(tos::devs::timer_t<2>) {
    return stm32::general_timer{stm32::detail::gen_timers[0]};
}

inline stm32::general_timer open_impl(tos::devs::timer_t<3>) {
    return stm32::general_timer{stm32::detail::gen_timers[1]};
}
} // namespace tos

// impl

namespace tos::stm32 {
inline general_timer::general_timer(const detail::gen_tim_def& def)
    : tracked_driver(std::distance(detail::gen_timers, &def))
    , m_def{&def}
    , m_fun{[](void*) {}} {
    m_def->rcc_en();

    m_handle = TIM_HandleTypeDef{};
    m_handle.Instance = def.tim;
    m_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    m_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    m_handle.Init.Period = 0xFFFF; // 65535
#if defined(STM32L4)
    m_handle.Init.RepetitionCounter = 0;
#endif
#if defined(STM32L0)
    m_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
#endif

    HAL_NVIC_SetPriority(m_def->irq, 0, 0);
}

inline void general_timer::set_frequency(uint16_t hertz) {
    m_handle.Init.Prescaler = apb1_clock / 1'000 - 1;
    m_handle.Init.Period = (2000 / hertz) - 1;
    auto init_res = HAL_TIM_Base_Init(&m_handle);
    if (init_res != HAL_OK) {
        // error
    }
}

inline void general_timer::enable() {
    HAL_NVIC_EnableIRQ(m_def->irq);
    tos::kern::busy();
    HAL_TIM_Base_Start_IT(&m_handle);
}

inline void general_timer::disable() {
    HAL_NVIC_DisableIRQ(m_def->irq);
    tos::kern::unbusy();
    HAL_TIM_Base_Stop_IT(&m_handle);
}

inline uint32_t general_timer::get_counter() const {
    return m_handle.Instance->CNT;
}

inline uint32_t general_timer::get_period() const {
    return m_handle.Init.Period;
}
} // namespace tos::stm32