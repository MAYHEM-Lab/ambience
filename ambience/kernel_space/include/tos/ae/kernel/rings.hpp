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
};

template<class ExecutorT>
inline void proc_req_queue(ExecutorT&& executor, kernel_interface& iface) {
    iface.req_last_seen =
        for_each(*iface.user_iface,
                 *iface.user_iface->guest_to_host,
                 iface.req_last_seen,
                 iface.user_iface->size,
                 [&iface, &executor](ring_elem& elem) {
                     auto& req = elem.req;

                     if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                         // Response for a request we made.

                         auto& res = elem.res;
                         if (res.user_ptr) {
                             auto& continuation =
                                 *static_cast<tos::function_ref<void()>*>(res.user_ptr);
                             continuation();
                         }
                     } else {
                         executor(req, [&elem, &iface] {
                             respond<true>(*iface.user_iface, elem);
                         });
                     }
                 });
}
} // namespace tos::ae::kernel