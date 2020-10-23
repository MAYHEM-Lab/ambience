#include <tos/scheduler.hpp>

namespace tos::kern{
void busy() {
    global::sched.busy++;
}

void unbusy() {
    global::sched.busy--;
}

exit_reason scheduler::schedule() {
//    if (global::thread_state.num_threads == 0) {
//        // no thread left, potentially a bug
//        return exit_reason::restart;
//    }

    /**
     * We must disable interrupts before we look at the run_queue and sc.busy.
     * An interrupt might occur between the former and the latter and we can
     * power down even though there's something to run.
     */
    tos::int_guard ig;
    if (m_run_queue.empty()) {
        /**
         * there's no thread to run right now
         */

        if (busy > 0) {
            return exit_reason::idle;
        }

        return exit_reason::power_down;
    }

    auto next_ctx = &m_run_queue.front().get_context();

    if (m_prev_context != next_ctx) {
        m_prev_context->switch_out(*next_ctx);
    }

    auto& front = m_run_queue.front();
    m_run_queue.pop_front();

    if (m_prev_context != next_ctx) {
        next_ctx->switch_in(*m_prev_context);
    }

    front();

    return exit_reason::yield;
}

void make_runnable(job& t) {
    global::sched.make_runnable(t);
}
}