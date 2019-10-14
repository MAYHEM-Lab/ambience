//#include <tos/ft.inl>

#include <tos/scheduler.hpp>
#include <tos/tcb.hpp>

namespace tos {
kern::scheduler sched;
namespace impl {
kern::tcb* cur_thread = nullptr;
}
} // namespace tos