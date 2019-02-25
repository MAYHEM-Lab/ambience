//
// Created by fatih on 8/25/18.
//

#pragma once

#include <chrono>

namespace tos
{
    void delay_us(std::chrono::microseconds us);
    void delay_ms(std::chrono::milliseconds ms);
}