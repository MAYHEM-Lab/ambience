#pragma once

#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/rings.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct awaiter {
    auto operator co_await() {
        g->notify_downcall();
        return m_elem->submit<true>(g->iface.user_iface, id, m_elem);
    }

    req_elem* m_elem;
    int id;
    kernel::user_group* g;
};

struct downcall_transport {
    explicit downcall_transport(kernel::user_group& g, int channel)
        : g{&g}
        , channel_id{channel} {
    }

    awaiter execute(int proc_id, const void* args, void* res) {
        const auto& [req, id] =
            prepare_req<true>(*g->iface.user_iface, channel_id, proc_id, args, res);

        return awaiter{
            .m_elem = &req,
            .id = id,
            .g = g,
        };
    }

    kernel::user_group* g;
    int channel_id;
};
} // namespace tos::ae