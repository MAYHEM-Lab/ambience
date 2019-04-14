//
// Created by fatih on 1/17/19.
//

#pragma once

#include <tos/span.hpp>
#include <tos/utility.hpp>
#include <common/driver_base.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>

namespace tos
{
namespace stm32
{
    namespace detail
    {
        struct adc_def
        {
            rcc_periph_clken rcc;
            uint32_t adc;
        };

        constexpr adc_def adcs[] = {
            { RCC_ADC1, ADC1 }
        };
    }

    class adc : public self_pointing<adc>, public non_copy_movable
    {
    public:

        explicit adc(const detail::adc_def* def) : m_def{def}
        {
            rcc_periph_clock_enable(m_def->rcc);

            adc_power_off(m_def->adc);

            adc_disable_scan_mode(m_def->adc);
            adc_set_single_conversion_mode(m_def->adc);
            adc_disable_external_trigger_regular(m_def->adc);
            adc_set_right_aligned(m_def->adc);

            adc_enable_temperature_sensor();
            adc_set_sample_time_on_all_channels(m_def->adc, ADC_SMPR_SMP_28DOT5CYC);

            adc_power_on(m_def->adc);
            for (int i = 0; i < 800'000; i++) __asm__("nop");

            adc_reset_calibration(m_def->adc);
            adc_calibrate(m_def->adc);
        }

        void set_channels(span<uint8_t> chs)
        {
            adc_set_regular_sequence(m_def->adc, chs.size(), chs.data());
        }

        uint32_t read()
        {
            adc_start_conversion_direct(m_def->adc);
            while (!adc_eoc(m_def->adc));
            return adc_read_regular(m_def->adc);
        }

        ~adc()
        {
            adc_power_off(m_def->adc);
            adc_disable_temperature_sensor();
            rcc_periph_clock_disable(m_def->rcc);
        }

    private:

        const detail::adc_def* m_def;
    };
}
}