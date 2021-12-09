#include <tos/ae/kernel/user_group.hpp>

extern tos::function_ref<void(tos::ae::kernel::group&)> make_runnable;
namespace tos::ae::kernel {
void user_group::notify_downcall() {
    //    tos::debug::log("Notify downcall", m_runnable, this);
    if (m_runnable)
        return;
    m_runnable = true;
    make_runnable(*this);
}

void user_group::clear_runnable() {
    //    tos::debug::log("Clear runnable", this);
    m_runnable = false;
}

int user_group::host_to_guest_queue_depth() const {
    return iface.user_iface->host_to_guest->size(iface.user_iface->size,
                                                 iface.user_iface->res_last_seen);
}
} // namespace tos::ae::kernel