//#include <tos/ft.inl>

#include <tos/tcb.hpp>
#include <tos/scheduler.hpp>

namespace tos
{
    kern::scheduler sched;
    namespace impl
    {
        kern::tcb* cur_thread = nullptr;
    }
}