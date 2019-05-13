//
// Created by fatih on 10/22/18.
//

#include <ctype.h>
#include <arch/cc.h>

extern "C"
{
int gettimeofday(struct timeval *t, void *timezone) {
  return -1;
}

int isxdigit(int c)
{
    auto in_range = [](auto c, auto lo, auto up)
    {
        return ((u8_t)c >= lo && (u8_t)c <= up);
    };

    return isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F');
}
}
