//
// Created by Mehmet Fatih BAKIR on 29/10/2018.
//

#pragma once

#include <chrono>
#include <tos/print.hpp>
#include <cwpack.hpp>

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

    float to_fahrenheits(float c)
    {
        if (c == -1) return -1;
        return c * 9 / 5 + 32;
    }

    template <class StreamT>
    void print(StreamT& str, uint32_t id, const sample& s)
    {
        std::array<char, 40> buf;
        tos::msgpack::packer p{buf};
        auto arr = p.insert_arr(4);

        arr.insert(id);
        arr.insert(s.temp);
        arr.insert(s.humid);
        arr.insert(s.cpu);

        str->write(p.get());
    }
}
