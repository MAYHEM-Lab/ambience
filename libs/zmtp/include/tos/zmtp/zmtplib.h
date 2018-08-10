
#pragma once

#include <stdint.h>

typedef uint8_t byte;

typedef struct {
    byte flags;             //  Must be zero
    byte size;              //  Size, 0 to 255 bytes
    byte data [255];        //  Message data
} zmtp_msg_t;
