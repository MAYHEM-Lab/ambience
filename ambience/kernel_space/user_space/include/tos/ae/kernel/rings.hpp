#pragma once

#include <string_view>
#include <tos/ae/rings.hpp>
#include <tos/debug/log.hpp>
#include <tos/flags.hpp>

namespace tos::ae::kernel {
/**
 * Instances of this type correspond to a ring in user space, and keep track of the kernel
 * specific data regarding that ring.
 */
struct kernel_interface {
    tos::ae::interface* user_iface;
    uint16_t req_last_seen = 0;

    int guest_to_host_queue_depth() const {
        return user_iface->guest_to_host_queue_depth(req_last_seen);
    }
};

template<class ExecutorT>
inline void proc_req_queue(ExecutorT&& executor, kernel_interface& iface) {
    iface.req_last_seen =
        for_each(*iface.user_iface,
                 *iface.user_iface->guest_to_host,
                 iface.req_last_seen,
                 [&iface, &executor](const ring_elem& elem) {
                     if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                         // Response for a request we made.
                         auto& continuation =
                             *static_cast<tos::function_ref<void()>*>(elem.res.user_ptr);
                         continuation();
                     } else {
                         executor(elem.req, [ptr = elem.req.user_ptr, &iface](uintptr_t status) {
                             respond<true>(*iface.user_iface, ptr, status);
                         });
                     }
                 });
}
} // namespace tos::ae::kernel