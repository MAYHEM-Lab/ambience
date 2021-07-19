#pragma once

#include <tos/ae/kernel/runners/group_runner.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/detail/poll.hpp>
#include <tos/late_constructed.hpp>
#include <tos/preemption.hpp>

namespace tos::ae {
struct preemptive_user_group_runner : group_runner {
    explicit preemptive_user_group_runner(function_ref<bool(kern::tcb&)> runner)
        : m_erased_runner{runner} {
    }

    void run(kernel::group& group) override {
        auto& user_group = static_cast<kernel::user_group&>(group);

//        tos::debug::log("Running", user_group.iface.user_iface);
        auto preempted = m_erased_runner(*user_group.state);
//        tos::debug::log("Out", user_group.iface.user_iface, preempted);
        user_group.clear_runnable();
        post_run(user_group);

        if (preempted) {
            user_group.notify_downcall();
        } else {
            if (user_group.iface.user_iface->res_last_seen !=
                user_group.iface.user_iface->host_to_guest->head_idx) {
                user_group.notify_downcall();
            }
        }
    }

    static void create(function_ref<bool(kern::tcb&)> runner) {
        m_instance.emplace(runner);
    }

    static preemptive_user_group_runner& instance() {
        return m_instance.get();
    }

private:
    void post_run(kernel::user_group& group) {
        proc_req_queue(
            [&](tos::ae::req_elem req, auto done) {
                if (req.channel == 0) [[unlikely]] {
                    if (req.ret_ptr) {
                        *((volatile bool*)req.ret_ptr) = true;
                    }

                    if (req.procid == 1) {
                        LOG(*(std::string_view*)req.arg_ptr);
                    }

                    group.notify_downcall();
                    return done();
                }

                if (group.exposed_services.size() <= req.channel - 1) [[unlikely]] {
                    LOG_ERROR("No such service!");
                    group.notify_downcall();
                    return done();
                }

                auto& channel = group.exposed_services[req.channel - 1];

                if (auto sync = get_if<tos::ae::sync_service_host>(&channel)) {
                    tos::launch(tos::alloc_stack, [&group, sync, done, req] {
                        sync->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr);
                        group.notify_downcall();
                        return done();
                    });
                    return;
                }

                auto async = get_if<tos::ae::async_service_host>(&channel);

                tos::coro::make_detached(
                    async->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr),
                    [&group, done] {
                        group.notify_downcall();
                        done();
                    });
            },
            group.iface);
    }

    static late_constructed<preemptive_user_group_runner> m_instance;
    function_ref<bool(kern::tcb&)> m_erased_runner;
};

inline late_constructed<preemptive_user_group_runner>
    preemptive_user_group_runner::m_instance;
} // namespace tos::ae