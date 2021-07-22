#pragma once

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

namespace lidl {
/*
 * Instances of this transport can expose a sync service as an async one
 */
struct sync_to_async_bridge {
    template<class ServiceT>
    explicit sync_to_async_bridge(ServiceT* serv)
        : impl{serv}
        , zerocopy_vtable{lidl::make_zerocopy_vtable<typename ServiceT::service_type>()} {
    }

    tos::Task<bool> execute(int proc, const void* arg, void* res) {
        tos::semaphore sem{0};
        tos::launch(tos::alloc_stack, [this, proc, arg, res, &sem] {
            run_zerocopy(proc, arg, res);
            sem.up();
        });
        co_await sem;
        co_return true;
    }

    bool run_zerocopy(int proc, const void* arg, void* res) const {
        return zerocopy_vtable[proc](*impl, arg, res);
    }

    lidl::sync_service_base* impl;
    lidl::zerocopy_vtable_t zerocopy_vtable;
};
} // namespace lidl