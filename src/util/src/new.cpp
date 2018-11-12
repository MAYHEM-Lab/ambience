//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#include <stdlib.h>
#include <new>

void operator delete (void* pt, size_t){
    free(pt);
}

void operator delete (void* pt)
{
    free(pt);
}

void operator delete[] (void* pt)
{
    free(pt);
}

void operator delete[] (void* pt, size_t)
{
    free(pt);
}

void* operator new(size_t sz)
{
    return malloc(sz);
}

void* operator new[](size_t sz)
{
    return malloc(sz);
}