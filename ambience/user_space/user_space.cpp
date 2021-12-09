#include "tos/ae/rings.hpp"
#include <tos/ae/user_space.hpp>
#include <tos/debug/log.hpp>
#include <tos/detail/poll.hpp>
#include <tos/function_ref.hpp>

extern tos::ae::interface iface;

namespace tos::ae {
void done_callback(void* ptr) {
    // tos::debug::log("Responding", &iface, ptr);
    respond<false>(iface, ptr);
}

void proc_res_queue(interface& iface) {
    iface.res_last_seen = for_each(
        iface, *iface.host_to_guest, iface.res_last_seen, [&](const ring_elem& elem) {
            if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                // Response for a request we made.
                // tos::debug::log("User ptr", &iface, elem.res.user_ptr);

                auto& continuation =
                    *static_cast<tos::function_ref<void()>*>(elem.res.user_ptr);
                continuation();
            } else {
                // tos::debug::log("Serving", &iface, elem.res.user_ptr);

                // We have a request to serve.
                dispatch_request(elem.req);
            }
        });
}
} // namespace tos::ae