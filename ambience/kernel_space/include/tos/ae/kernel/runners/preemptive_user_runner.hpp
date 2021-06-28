#pragma once

#include <tos/ae/kernel/runners/group_runner.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/detail/poll.hpp>
#include <tos/preemption.hpp>

namespace tos::ae {
struct preemptive_user_group_runner : group_runner {
    explicit preemptive_user_group_runner(function_ref<bool(kern::tcb&, int)> runner)
        : m_erased_runner{runner} {
    }

    void run(kernel::group& group) override {
        auto& user_group = static_cast<kernel::user_group&>(group);
        m_erased_runner(*user_group.state, 5);
        post_run(user_group);
    }

private:
    void post_run(kernel::user_group& group) {
        proc_req_queue(
            [&](tos::ae::req_elem& req, auto done) {
                if (req.channel == 0) {
                    if (req.ret_ptr) {
                        *((volatile bool*)req.ret_ptr) = true;
                    }

                    if (req.procid == 1) {
                        LOG(*(std::string_view*)req.arg_ptr);
                    }

                    return done();
                }

                if (group.exposed_services.size() <= req.channel - 1) {
                    LOG_ERROR("No such service!");
                    return done();
                }

                auto& channel = group.exposed_services[req.channel - 1];

                if (auto sync = get_if<tos::ae::sync_service_host>(&channel)) {
                    tos::launch(tos::alloc_stack, [sync, done, req] {
                        sync->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr);
                        return done();
                    });
                    return;
                }

                auto async = get_if<tos::ae::async_service_host>(&channel);

                tos::coro::make_detached(
                    async->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr), done);
            },
            group.iface);
    }


    function_ref<bool(kern::tcb&, int)> m_erased_runner;
};
} // namespace tos::ae