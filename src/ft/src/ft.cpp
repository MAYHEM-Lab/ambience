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
} // namespace tos