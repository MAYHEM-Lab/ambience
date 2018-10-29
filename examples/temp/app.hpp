//
// Created by Mehmet Fatih BAKIR on 29/10/2018.
//

#pragma once

namespace temp
{
struct sample
{
    float temp;
    float humid;
};

static_assert(sizeof(sample) == 8, "");
}
