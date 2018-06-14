#pragma once

namespace tos {
    /**
     * Places the given thread back into the runnable list
     * @param t thread to make runnable
     */
    void make_runnable(struct tcb &t);

    /**
     * Gives control of the CPU back to the scheduler, suspending
     * the current thread.
     *
     * If the interrupts are not disabled when this function
     * is called, the behaviour is undefined
     */
    void suspend_self();
    // pre-condition: interrupts must be disabled

    void busy();
    void unbusy();
}

