#pragma once

#include <stddef.h>
#include <stdint.h>

#define TOS_COND_SIZE      4 * sizeof(uintptr_t)
#define TOS_COND_ALIGNMENT _Alignof(max_align_t)

#if defined(__cplusplus)
extern "C" {
#endif

struct pthread_cond_t {
    _Alignas(TOS_COND_ALIGNMENT) char cond_buffer[TOS_COND_SIZE];
};

#define PTHREAD_COND_INITIALIZER { { } }

typedef void* pthread_condattr_t;

#if defined(__cplusplus)
}
#endif
