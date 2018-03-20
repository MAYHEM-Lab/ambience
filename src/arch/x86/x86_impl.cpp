//
// Created by fatih on 3/20/18.
//

#include "tos/atomic.hpp"
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"

#include <string.h>
#include <stdlib.h>

extern "C"
{
    /*void tos_set_stack_ptr(void* ptr)
    {
        // return address is in the stack
        // if we just change the stack pointer
        // we can't return to the caller
        // copying the last 10 bytes from the original
        // stack to this stack so that we'll be able
        // to return

        uintptr_t sp;
        asm("movq %%rsp, %0" : "=r"(sp) :);
        memcpy(ptr - 10, (void*)sp, 10);
        sp = reinterpret_cast<uintptr_t >(ptr - 10);
        asm("movq %0, %%rsp" : : "r"(sp) : "%rsp");
    }*/

    void tos_power_down()
    {
    }

    void* tos_stack_alloc(size_t size)
    {
        return aligned_alloc(16, size);
    }

    void tos_stack_free(void* data)
    {
        free(data);
    }

    void tos_shutdown()
    {
        exit(1);
    }
}


namespace tos
{
    template <class T>
    void atomic<T>::add(const T& t)
    {
        asm("lock addl %0, (%1)": : "r"(t), "r"(&m_t) : "memory");
        //m_t += t;
    }

    //template class atomic<char>;
    //template class atomic<short>;
    template class atomic<int>;
    //template class atomic<unsigned int>;
    //template class atomic<long>;
}

// the optional arduino yield thing
extern "C" void yield()
{
    if (!ft::self())
    {
        return;
    }
    ft::this_thread::yield();
}
