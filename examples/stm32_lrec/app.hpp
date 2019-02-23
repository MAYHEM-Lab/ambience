//
// Created by Mehmet Fatih BAKIR on 29/10/2018.
//

#pragma once

#include <chrono>
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

float to_f(float c)
{
    if (c == -1) return -1;
    return c * 9 / 5 + 32;
}

template <class StreamT>
void print(StreamT& str, int id, const sample& s)
{
    tos::print(str, -999, "");
    tos::print(str, id, "");
    tos::print(str, 0, 0, "");
    tos::print(str, 1, int(to_f(s.temp)), "");
    tos::print(str, 2, int(s.humid), "");
    tos::print(str, 3, int(to_f(s.cpu)), "");
    tos::println(str);
}
}
