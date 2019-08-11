//
// Created by fatih on 10/22/18.
//

#include <ctype.h>
#include <arch/cc.h>
#include <cstdlib>
#include <tos/thread.hpp>

extern "C"
{
int gettimeofday(struct timeval *, void *) {
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

void _exit()
{
    tos::this_thread::exit();
}
_PTR
_malloc_r (struct _reent *, size_t sz)
{
    return malloc (sz);
}

void
_free_r (struct _reent *, void* ptr)
{
    free(ptr);
}

_PTR
_realloc_r(struct _reent *, void* ptr, size_t sz)
{
    return realloc(ptr, sz);
}

int
_fstat_r (struct _reent *, int, struct stat *pstat)
{
    memset(&pstat, 0, sizeof(pstat));
    return 0;
}

void _getpid_r() {}
void _kill_r() {}
}
