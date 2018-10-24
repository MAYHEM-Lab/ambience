//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tos/chrono.hpp>
#include <utility>

namespace tos {
    template<class T> using invoke_t = typename T::type;

    template<class S1, class S2>
    struct concat;

    template<class T, T... I1, T... I2>
    struct concat<std::integer_sequence<T, I1...>, std::integer_sequence<T, I2...>>
    : std::integer_sequence<T, I1..., (sizeof...(I1)+I2)...>
    {
    };

    template<class S1, class S2>
    using concat_t = invoke_t<concat<S1, S2>>;

    template <class AlarmT, class FunT>
    void forever(AlarmT& alarm, milliseconds ms, FunT&& fun)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (true)
        {
            std::forward<FunT>(fun)();
            alarm.sleep_for(ms);
        }
#pragma clang diagnostic pop
    }
}

