#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>

void post(tos::coro::pollable p);
extern tos::ae::interface iface;
tos::Task<bool> handle_req(tos::ae::req_elem el);

namespace tos::ae {
tos::Task<void> log_str(std::string_view sv) {
    co_await tos::ae::submit_req(iface, 4, 1, &sv, nullptr);
}

void proc_res_queue(interface& iface) {
    // TODO: handle overflow
    for (; iface.res_last_seen < iface.res->head_idx; ++iface.res_last_seen) {
        auto res_idx = iface.res->elems[iface.res_last_seen % iface.size];

        auto& elem = iface.elems[res_idx];

        if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
            // Response for a request we made.

            auto& res = elem.res;
            std::coroutine_handle<>::from_address(res.user_ptr).resume();
        } else {
            // We have a request to serve.
            auto& req = elem.req;
            auto handler = tos::coro::make_pollable(handle_req(req));
            if (!handler.run()) {
                post(std::move(handler));
            }
        }
    }
}
} // namespace tos::ae