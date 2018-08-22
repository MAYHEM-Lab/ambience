//
// Created by fatih on 8/22/18.
//

extern "C"
{
#include <user_interface.h>
#include <mem.h>
}

#include <tos/ft.hpp>

extern "C"
{
void* malloc(size_t sz)
{
    return os_malloc(sz);
}

void free(void* ptr)
{
    os_free(ptr);
}

void* calloc(size_t nitems, size_t size)
{
    return os_zalloc(nitems * size);
}

void* realloc(void* base, size_t sz)
{
    return os_realloc(base, sz);
}

void ax_wdt_feed()
{
    system_soft_wdt_feed();
    //tos::this_thread::yield();
}

void _exit()
{
    tos::this_thread::exit();
}
_PTR
_malloc_r (struct _reent *r, size_t sz)
{
    return malloc (sz);
}

void
_free_r (struct _reent *r, void* ptr)
{
    free(ptr);
}

_PTR
_realloc_r(struct _reent *r, void* ptr, size_t sz)
{
    return realloc(ptr, sz);
}

int
_fstat_r (struct _reent *ptr, int fd, struct stat *pstat)
{
    memset(&pstat, 0, sizeof(pstat));
    return 0;
}

void _getpid_r() {}
void _kill_r() {}
}
