#pragma once

#include <tos/ae/rings.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct upcall_transport {
    explicit upcall_transport(interface& iface, int channel)
        : iface{&iface}
        , channel_id{channel} {
    }

    tos::Task<bool> execute(int proc_id, const void* args, void* res) {
        co_await tos::ae::submit_req<false>(*iface, channel_id, proc_id, args, res);
        co_return true;
    }

    interface* iface;
    int channel_id;
};
} // namespace tos::ae