#include <tos/detail/tos_bind.hpp>
#include <tos/ft.hpp>
#include <tos/scheduler.hpp>
#include <tos/tcb.hpp>

namespace tos {
namespace global {
threading_state thread_state;
}
tos::context& current_context() {
    if (!self())
        return default_context();
    return self()->get_context();
}

namespace kern {
deleter::~deleter() {
    /**
     * We free the stack later here because the tcb potentially lives in the memory we
     * are deleting here, and there are still other destructors to run.
     */
    auto lambda = [](char* ptr) -> tos::coro::detached {
        co_await tos::coro::yield();
        delete[] ptr;
    };
    lambda(get_task_base());
}
} // namespace kern
} // namespace tos