#pragma once

#include "thread.hpp"

#include <setjmp.h>
#include <stdint.h>
#include <tos/interrupt.hpp>
#include <tos/tcb.hpp>

namespace tos {

/**
 * This enumeration denotes why the scheduler returned
 */
enum class exit_reason
{
    /**
     * All threads are blocked, however there are
     * busy threads. System should wait for any interrupt.
     */
    idle,
    /**
     * All threads are blocked, and the system isn't busy.
     * We can go to the lowest possible power state.
     */
    power_down,
    /**
     * All threads exited. Usually denotes a bug, system
     * should restart.
     */
    restart,
    /**
     * Threading subsystem yields the CPU back to the
     * architecture. This is useful for architectures that
     * require periodic control over CPU.
     *
     * If the architecture doesn't mandate such a requirement
     * this value may never be returned.
     */
    yield
};

namespace kern {
/**
 * Places the given thread back into the runnable list
 * @param t thread to make runnable
 */
void make_runnable(struct tcb& t);

/**
 * Gives control of the CPU back to the scheduler, suspending
 * the current thread.
 *
 * If the interrupts are not disabled when this function
 * is called, the behaviour is undefined
 */
void suspend_self(const no_interrupts&);
// pre-condition: interrupts must be disabled

/**
 * Keeps the CPU from going into deep sleep. Must be used
 * before blocking on something that doesn't emit a wake up
 * capable interrupt.
 */
void busy();

/**
 * Undoes the operation done with the associated `busy` call.
 * Every busy call must be matched with one and only one `unbusy`
 * call.
 *
 * The CPU may go to sleep _immediately_ after executing this
 * function.
 */
void unbusy();
} // namespace kern
} // namespace tos


namespace tos {
namespace kern {
class scheduler {
public:
    template<class TaskT>
    thread_id_t start(TaskT&);

    /**
     * This function schedules _one_ thread execution and returns.
     *
     * When all threads block, it will return a reason explaining
     * why the return happened.
     *
     * Depending on the reason, implementations should enter a
     * power saving state accordingly.
     *
     * Behaviour is undefined if this function is called from a
     * thread context.
     *
     * @return exit reason
     */
    exit_reason schedule();

    void make_runnable(tcb& task) {
        m_run_queue.push_back(task);
    }

    ctx main_context{};
    uint8_t busy = 0;

private:
    int8_t num_threads = 0;

    intrusive_list<tcb> m_run_queue;
};
} // namespace kern
} // namespace tos
