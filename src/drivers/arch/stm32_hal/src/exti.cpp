//
// Created by fatih on 9/12/19.
//

#include <arch/exti.hpp>
#include <stm32_hal/gpio.hpp>

#if defined(STM32L0)
template <size_t ExtiNum>
void do_exti_isr()
{
    if ((EXTI->PR & ExtiNum) == ExtiNum)
    {
        EXTI->PR = ExtiNum;
        auto it = tos::stm32::exti::get(0);
        if (!it)
        {
            tos::debug::panic("No EXTI driver instance!");
        }
        it->isr(ExtiNum);
    }
}

extern "C"
{
void EXTI0_1_IRQHandler()
{
    do_exti_isr<GPIO_PIN_0>();
    do_exti_isr<GPIO_PIN_1>();
}
void EXTI2_3_IRQHandler()
{
    do_exti_isr<GPIO_PIN_2>();
    do_exti_isr<GPIO_PIN_3>();
}
void EXTI4_15_IRQHandler()
{
    do_exti_isr<GPIO_PIN_4>();
    do_exti_isr<GPIO_PIN_5>();
    do_exti_isr<GPIO_PIN_6>();
    do_exti_isr<GPIO_PIN_7>();
    do_exti_isr<GPIO_PIN_8>();
    do_exti_isr<GPIO_PIN_9>();
    do_exti_isr<GPIO_PIN_10>();
    do_exti_isr<GPIO_PIN_11>();
    do_exti_isr<GPIO_PIN_12>();
    do_exti_isr<GPIO_PIN_13>();
    do_exti_isr<GPIO_PIN_14>();
    do_exti_isr<GPIO_PIN_15>();
}
}
#elif defined(STM32L4)
template <size_t ExtiNum>
void do_exti_isr()
{
    if ((EXTI->PR1 & ExtiNum) == ExtiNum)
    {
        EXTI->PR1 = ExtiNum;
        auto it = tos::stm32::exti::get(0);
        if (!it)
        {
            tos::debug::panic("No EXTI driver instance!");
        }
        it->isr(ExtiNum);
    }
}

extern "C"
{
void EXTI0_IRQHandler() {
    do_exti_isr<GPIO_PIN_0>();
}
void EXTI1_IRQHandler() {
    do_exti_isr<GPIO_PIN_1>();
}
void EXTI2_IRQHandler() {
    do_exti_isr<GPIO_PIN_2>();
}
void EXTI3_IRQHandler() {
    do_exti_isr<GPIO_PIN_3>();
}
void EXTI4_IRQHandler() {
    do_exti_isr<GPIO_PIN_4>();
}
void EXTI9_5_IRQHandler()
{
    do_exti_isr<GPIO_PIN_5>();
    do_exti_isr<GPIO_PIN_6>();
    do_exti_isr<GPIO_PIN_7>();
    do_exti_isr<GPIO_PIN_8>();
    do_exti_isr<GPIO_PIN_9>();
}
void EXTI15_10_IRQHandler()
{
    do_exti_isr<GPIO_PIN_10>();
    do_exti_isr<GPIO_PIN_11>();
    do_exti_isr<GPIO_PIN_12>();
    do_exti_isr<GPIO_PIN_13>();
    do_exti_isr<GPIO_PIN_14>();
    do_exti_isr<GPIO_PIN_15>();
}
}
#endif
