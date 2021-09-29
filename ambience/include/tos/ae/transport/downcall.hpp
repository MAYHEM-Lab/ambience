#pragma once

#include <tos/ae/kernel/as_traversal.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/rings.hpp>
#include <tos/debug/debug.hpp>
#include <tos/meta/types.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct downcall_transport {
    template<class ServiceT>
    explicit downcall_transport(meta::id<ServiceT>, kernel::user_group& g, int channel)
        : g{&g}
        , channel_id{channel}
        , ipc_area_vtbl(make_downcall_sharer<ServiceT>()) {
    }

    auto execute(int proc_id, const void* args, void* res) {
        struct awaiter {
            auto operator co_await() {
                g->notify_downcall();
                return m_elem->submit<true>(g->iface.user_iface, id, m_elem);
            }

            req_elem* m_elem;
            int id;
            kernel::user_group* g;
            std::unique_ptr<quik::share_base> keep_args_alive;
        };

        auto translated_args =
            ipc_area_vtbl[proc_id].do_share(*tos::global::cur_as, *g->as, args, res);
        //        tos::debug::log("Mapped args from", args, "to", translated_args.get());
        const auto& [req, id] = prepare_req<true>(*g->iface.user_iface,
                                                  channel_id,
                                                  proc_id,
                                                  translated_args->get_tuple_ptr(),
                                                  res);

        //      auto ipc_size = ipc_area_vtbl[proc_id](args);
        //    auto region = new uint8_t[ipc_size];
        //  tos::debug::do_not_optimize(region);
        // delete[] region;

        return awaiter{.m_elem = &req,
                       .id = id,
                       .g = g,
                       .keep_args_alive = std::move(translated_args)};
    }

    kernel::user_group* g;
    int channel_id;
    tos::span<const sharer_vtbl> ipc_area_vtbl;
};
} // namespace tos::ae
