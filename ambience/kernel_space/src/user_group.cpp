#include <tos/ae/kernel/user_group.hpp>

extern tos::function_ref<void(tos::ae::kernel::group&)> make_runnable;
namespace tos::ae::kernel {
void user_group::notify_downcall() {
//    tos::debug::log("Notify downcall", m_runnable);
    if (m_runnable)
        return;
    m_runnable = true;
    make_runnable(*this);
}
} // namespace tos::ae::kernel