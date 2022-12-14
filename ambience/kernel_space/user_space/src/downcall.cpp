#include "tos/ae/rings.hpp"
#include <tos/ae/transport/downcall.hpp>

namespace tos::ae {
downcall_transport::downcall_transport(tos::span<const sharer_vtbl> vtbl,
                                       kernel::user_group& g,
                                       int channel)
    : g{&g}
    , channel_id{channel}
    , ipc_area_vtbl(vtbl) {
}

auto downcall_transport::awaiter::operator co_await() -> req_elem::awaiter<true> {
    if (!m_elem) {
        return req_elem::awaiter<true>{.el = nullptr};
    }

    transport->g->notify_downcall();
    return m_elem->submit<true>(transport->g->iface.user_iface, id, m_elem);
}

auto downcall_transport::execute(int proc_id, const void* args, void* res) -> awaiter {
    auto translated_args =
        ipc_area_vtbl[proc_id].do_share(*tos::global::cur_as, *g->as, args, res);

    auto ring_res = prepare_req(
        *g->iface.user_iface, channel_id, proc_id, translated_args->get_tuple_ptr(), res);

    if (!ring_res) {
        tos::debug::error("Ring allocation failed");
        return awaiter{
            .m_elem = nullptr
        };
    }

    const auto& [req, id] = force_get(ring_res);

    return awaiter{.m_elem = req,
                   .id = id,
                   .transport = this,
                   .orig_args = args,
                   .proc_id = proc_id,
                   .keep_args_alive = std::move(translated_args)};
}

downcall_transport::awaiter::~awaiter() {
    transport->ipc_area_vtbl[proc_id].finalize(*keep_args_alive, orig_args);
}
} // namespace tos::ae