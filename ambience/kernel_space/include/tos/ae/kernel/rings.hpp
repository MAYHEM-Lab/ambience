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

inline void proc_req_queue(kernel_interface& iface) {
    iface.req_last_seen =
        for_each(*iface.user_iface,
                 *iface.user_iface->req,
                 iface.req_last_seen,
                 iface.user_iface->size,
                 [&iface](ring_elem& elem) {
                     auto& req = elem.req;
                     LOG(req.channel, req.procid, req.arg_ptr, req.user_ptr, req.ret_ptr);

                     if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                         // Response for a request we made.

                         auto& res = elem.res;
                         if (res.user_ptr) {
                             std::coroutine_handle<>::from_address(res.user_ptr).resume();
                         }
                     } else {
                         if (req.ret_ptr) {
                             *((volatile bool*)req.ret_ptr) = true;
                         }

                         if (req.channel == 4 && req.procid == 1) {
                             LOG(iface.req_last_seen,
                                 iface.user_iface->res->head_idx,
                                 *(std::string_view*)req.arg_ptr);
                         }

                         respond<true>(*iface.user_iface, elem);
                     }
                 });
}
} // namespace tos::ae::kernel