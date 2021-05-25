#pragma once

#include <tos/ae/rings.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct awaiter {
    auto operator co_await() {
        tos::ae::submit_elem<true>(*iface, id);
        return m_elem->operator co_await();
    }

    req_elem* m_elem;
    int id;
    interface* iface;
};

struct downcall_transport {
    explicit downcall_transport(interface& iface, int channel)
        : iface{&iface}
        , channel_id{channel} {
    }

    awaiter execute(int proc_id, const void* args, void* res) {
        const auto& [req, id] = prepare_req(*iface, channel_id, proc_id, args, res);

        return awaiter{
            .m_elem = &req,
            .id = id,
            .iface = iface,
        };
    }

    interface* iface;
    int channel_id;
};
} // namespace tos::ae