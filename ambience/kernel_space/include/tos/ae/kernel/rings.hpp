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
        for_each(*iface.user_iface->req,
                 iface.req_last_seen,
                 iface.user_iface->size,
                 [&iface](uint16_t req_idx) {
                     auto& req = iface.user_iface->elems[req_idx].req;
                     LOG(req.user_ptr, req.arg_ptr);

                     if (req.channel == 4 && req.procid == 1) {
                         LOG(iface.req_last_seen,
                             iface.user_iface->res->head_idx,
                             *(std::string_view*)req.arg_ptr);
                     }

                     respond<true>(*iface.user_iface, iface.user_iface->elems[req_idx]);
                 });
}
} // namespace tos::ae::kernel