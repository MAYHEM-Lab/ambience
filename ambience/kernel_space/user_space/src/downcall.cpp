#include "tos/ae/rings.hpp"
#include <tos/ae/transport/downcall.hpp>

namespace tos::ae {
auto downcall_transport::awaiter::operator co_await() -> req_elem::awaiter<true> {
    transport->g->notify_downcall();
    return m_elem->submit<true>(transport->g->iface.user_iface, id, m_elem);
}

auto downcall_transport::execute(int proc_id, const void* args, void* res) -> awaiter {
    auto translated_args =
        ipc_area_vtbl[proc_id].do_share(*tos::global::cur_as, *g->as, args, res);

    const auto& [req, id] = prepare_req<true>(
        *g->iface.user_iface, channel_id, proc_id, translated_args->get_tuple_ptr(), res);

    return awaiter{.m_elem = &req,
                   .id = id,
                   .transport = this,
                   .orig_args = args,
                   .keep_args_alive = std::move(translated_args)};
}

downcall_transport::awaiter::~awaiter() {
    transport->ipc_area_vtbl[m_elem->procid].finalize(*keep_args_alive, orig_args);
}
} // namespace tos::ae