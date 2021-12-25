#pragma once

#include <stddef.h>
#include <stdint.h>

#define TOS_RW_SIZE      8 * sizeof(uintptr_t)
#define TOS_RW_ALIGNMENT _Alignof(max_align_t)
