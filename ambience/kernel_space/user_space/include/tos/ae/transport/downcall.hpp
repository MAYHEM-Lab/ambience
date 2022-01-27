#pragma once

#include <tos/ae/kernel/as_traversal.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/rings.hpp>
#include <tos/debug/debug.hpp>
#include <tos/meta/types.hpp>
#include <tos/quik.hpp>
#include <tos/task.hpp>

namespace tos::ae {
struct downcall_transport {
    explicit downcall_transport(tos::span<const sharer_vtbl> vtbl,
                                kernel::user_group& g,
                                int channel);

    struct awaiter {
        req_elem::awaiter<true> operator co_await();

        ~awaiter();

        req_elem* m_elem;
        int id;
        downcall_transport* transport;
        const void* orig_args;
        int proc_id;
        std::unique_ptr<quik::share_base> keep_args_alive;
    };

    awaiter execute(int proc_id, const void* args, void* res);

    kernel::user_group* g;
    int channel_id;
    tos::span<const sharer_vtbl> ipc_area_vtbl;
};

using downcall_factory_fn_t = downcall_transport (*)(kernel::user_group& g, int channel);

template<class ServiceT>
downcall_factory_fn_t make_downcall_factory() {
    return +[](kernel::user_group& g, int channel) {
        return downcall_transport(make_downcall_sharer<ServiceT>(), g, channel);
    };
}
} // namespace tos::ae
