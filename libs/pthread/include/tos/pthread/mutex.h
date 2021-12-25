#pragma once

#include <stddef.h>
#include <stdint.h>

#define TOS_MUTEX_SIZE      4 * sizeof(uintptr_t)
#define TOS_MUTEX_ALIGNMENT _Alignof(max_align_t)
