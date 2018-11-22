//
// Created by Mehmet Fatih BAKIR on 29/10/2018.
//

#pragma once

#include <chrono>
#include <avr/io.h>
#include <tos/print.hpp>

namespace temp
{

    static constexpr auto master_id = 101;
    static constexpr auto slave_id = 102;

struct sample
{
    float temp;
    float humid;
    float cpu;
};

template <class StreamT>
void print(StreamT& str, int id, const sample& s)
{
    static char fl_buf[10];
    tos::print(str, id, "");
    tos::print(str, 1, dtostrf(s.temp, 2, 2, fl_buf), "");
    tos::print(str, 2, dtostrf(s.humid, 2, 2, fl_buf), "");
    tos::println(str, 3, dtostrf(s.cpu, 2, 2, fl_buf));
}

static_assert(sizeof(sample) == 12, "");


    template <class AlarmT>
    double GetTemp(AlarmT&& alarm)
    {
        ADMUX = (3 << REFS0) | (8 << MUX0); // 1.1V REF, channel#8 is temperature
        ADCSRA |= (1 << ADEN) | (6 << ADPS0);       // enable the ADC div64

        using namespace std::chrono_literals;
        alarm.sleep_for(20ms);

        ADCSRA |= (1 << ADSC);      // Start the ADC

        while (ADCSRA & (1 << ADSC));       // Detect end-of-conversion

        // The offset of 324.31 could be wrong. It is just an indication.
        // The returned temperature is in degrees Celcius.
        return (ADCW - 324.31) / 1.22;
    }
}
