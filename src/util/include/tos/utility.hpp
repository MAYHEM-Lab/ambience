//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tos/chrono.hpp>
#include <utility>

namespace tos {
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

