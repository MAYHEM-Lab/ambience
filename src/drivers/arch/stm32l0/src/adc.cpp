//
// Created by fatih on 4/11/19.
//


#include <libopencm3/stm32/adc.h>
#include <arch/adc.hpp>

/*void adc1_2_isr()
{
    adc_get_flag(ADC1, ADC_SR_EOC);
    adc_clear_flag(ADC1, ADC_SR_EOC);
    if ((ADC_SR(ADC1) & ADC_SR_EOC) == ADC_SR_EOC)
    {
        auto val = adc_read_regular(ADC1);
        //tos::stm32::adc;
    }
}*/
