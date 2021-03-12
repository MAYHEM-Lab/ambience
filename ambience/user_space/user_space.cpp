#include <tos/ae/user_space.hpp>

extern tos::ae::interface iface;

namespace tos::ae {
tos::Task<void> log_str(std::string_view sv) {
    co_await tos::ae::submit_req(iface, 4, 1, &sv, nullptr);
}

void proc_res_queue(interface& iface) {
    for (; iface.res_last_seen < iface.res->head_idx; ++iface.res_last_seen) {
        auto res_idx = iface.res->elems[iface.res_last_seen % iface.size];
        auto& res = iface.elems[res_idx].res;
        std::coroutine_handle<>::from_address(res.user_ptr).resume();
    }
}
} // namespace tos::ae