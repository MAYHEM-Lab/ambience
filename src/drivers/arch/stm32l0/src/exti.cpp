//
// Created by fatih on 7/24/19.
//

#include <arch/exti.hpp>
#include <libopencm3/stm32/exti.h>

template <size_t ExtiNum>
void do_exti_isr()
{
    if ((EXTI_PR & ExtiNum) == ExtiNum)
    {
        EXTI_PR = ExtiNum;
        auto it = tos::stm32::exti::get(0);
        if (!it)
        {
            tos::kern::fatal("No EXTI driver instance!");
        }
        it->isr(ExtiNum);
    }
}

extern "C" {
#ifdef STM32L4
    void exti0_isr()
    {
        do_exti_isr<EXTI0>();
    }
    void exti1_isr()
    {
        do_exti_isr<EXTI1>();
    }
    void exti2_isr()
    {
        do_exti_isr<EXTI2>();
    }
    void exti3_isr()
    {
        do_exti_isr<EXTI3>();
    }
    void exti4_isr()
    {
        do_exti_isr<EXTI4>();
    }

    void exti9_5_isr()
    {
        do_exti_isr<EXTI5>();
        do_exti_isr<EXTI6>();
        do_exti_isr<EXTI7>();
        do_exti_isr<EXTI8>();
        do_exti_isr<EXTI9>();
    }
    void exti15_10_isr()
    {
        do_exti_isr<EXTI10>();
        do_exti_isr<EXTI11>();
        do_exti_isr<EXTI12>();
        do_exti_isr<EXTI13>();
        do_exti_isr<EXTI14>();
        do_exti_isr<EXTI15>();
    }
#elif defined(STM32L0)
    void exti0_1_isr()
    {
        do_exti_isr<EXTI0>();
        do_exti_isr<EXTI1>();
    }
    void exti2_3_isr()
    {
        do_exti_isr<EXTI2>();
        do_exti_isr<EXTI3>();
    }
    void exti4_15_isr()
    {
        do_exti_isr<EXTI4>();
        do_exti_isr<EXTI5>();
        do_exti_isr<EXTI6>();
        do_exti_isr<EXTI7>();
        do_exti_isr<EXTI8>();
        do_exti_isr<EXTI9>();
        do_exti_isr<EXTI10>();
        do_exti_isr<EXTI11>();
        do_exti_isr<EXTI12>();
        do_exti_isr<EXTI13>();
        do_exti_isr<EXTI14>();
        do_exti_isr<EXTI15>();
    }
#endif
}