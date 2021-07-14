#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>
#include <tos/function_ref.hpp>

extern tos::ae::interface iface;
tos::Task<bool> handle_req(tos::ae::req_elem el);

namespace tos::ae {
void proc_res_queue(interface& iface) {
    iface.res_last_seen =
        for_each(iface,
                 *iface.host_to_guest,
                 iface.res_last_seen,
                 iface.size,
                 [&](ring_elem& elem) {
                     if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                         // Response for a request we made.
                         auto& continuation =
                             *static_cast<tos::function_ref<void()>*>(elem.res.user_ptr);
                         continuation();
                     } else {
                         // We have a request to serve.
                         tos::coro::make_detached(handle_req(elem.req),
                                                  [&iface, ptr = elem.req.user_ptr]() {
                                                      respond<false>(iface, ptr);
                                                  });
                     }
                 });
}
} // namespace tos::ae