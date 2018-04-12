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
