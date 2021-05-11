#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>

void post(tos::coro::pollable p);
extern tos::ae::interface iface;
tos::Task<bool> handle_req(tos::ae::req_elem el);

namespace tos::ae {
tos::Task<void> log_str(std::string_view sv) {
    co_await tos::ae::submit_req<false>(iface, 0, 1, &sv, nullptr);
}

namespace {
tos::Task<void> req_adaptor(interface& iface, ring_elem& elem, tos::Task<bool> task) {
    auto res = co_await task;

    respond<false>(iface, elem);
}
} // namespace

void proc_res_queue(interface& iface) {
    iface.res_last_seen = for_each(
        iface, *iface.res, iface.res_last_seen, iface.size, [&](ring_elem& elem) {
            if (!util::is_flag_set(elem.common.flags, elem_flag::req)) {
                // Response for a request we made.

                auto& res = elem.res;
                std::coroutine_handle<>::from_address(res.user_ptr).resume();
            } else {
                // We have a request to serve.
                auto& req = elem.req;
                auto handler =
                    tos::coro::make_pollable(req_adaptor(iface, elem, handle_req(req)));
                if (!handler.run()) {
                    post(std::move(handler));
                }
            }
        });
}
} // namespace tos::ae