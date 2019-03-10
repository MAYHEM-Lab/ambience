//
// Created by fatih on 10/22/18.
//

#include <ctype.h>
#include <arch/cc.h>

extern "C"
{
#include <mem.h>
/*void *malloc(size_t sz) {
    return os_malloc(sz);
}

void free(void *ptr) {
    os_free(ptr);
}

void* calloc(size_t nitems, size_t size)
{
    return os_zalloc(nitems * size);
}*/
}

extern "C"
{
int isxdigit(int c)
{
    auto in_range = [](auto c, auto lo, auto up)
    {
        return ((u8_t)c >= lo && (u8_t)c <= up);
    };

    return isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F');
}
}
