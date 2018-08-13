//
// Created by fatih on 8/9/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    namespace did
    {
        struct unique_id_t
        {
            uint64_t id;
        };

        extern unique_id_t unique_id;

        static constexpr unique_id_t undefined_id{ 0 };
    }
}