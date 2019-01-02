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

template <class StreamT>
void print(StreamT& str, int id, const sample& s)
{
    tos::print(str, -999, "");
    tos::print(str, id, "");
    tos::print(str, 0, 0, "");
    auto string = std::to_string(s.temp);
    tos::print(str, 1, tos::span<const char>(string.data(), string.size()), "");
    string = std::to_string(s.humid);
    tos::print(str, 2, tos::span<const char>(string.data(), string.size()), "");
    string = std::to_string(s.cpu);
    tos::print(str, 3, tos::span<const char>(string.data(), string.size()), "");
}
}
