#pragma once

namespace tos {
    namespace kern
    {
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
    }
}

