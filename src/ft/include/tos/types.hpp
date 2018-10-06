//
// Created by fatih on 10/5/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    /**
     * This struct represents a unique identifier for every
     * **running** task in the system.
     */
    struct thread_id_t { uintptr_t id; };
}
