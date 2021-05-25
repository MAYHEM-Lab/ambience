//
// Created by fatih on 10/5/18.
//

#pragma once

#include <stddef.h>
#include <tos/ct_map.hpp>
#include <tos/interrupt.hpp>
#include <tos/tcb.hpp>
#include <tos/types.hpp>

namespace tos {
namespace this_thread {
/**
 * Returns a platform dependent identifier of the current task.
 *
 * Returns a defined, but unspecified value if this function is
 * called from a non-thread context.
 */
thread_id_t get_id();

/**
 * Give control of the CPU back to the scheduler. This will leave
 * the thread in a runnable state.
 *
 * Behaviour is undefined if this function is called from a
 * non-thread context.
 *
 * A blocking mechanism should always be preferred over yielding
 * in a busy wait.
 */
void yield(const no_interrupts& = tos::int_guard{});

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
 * Puts the calling thread to sleep for at least the given duration using the alarm
 * provided.
 *
 * This function is _not_ going to busy wait even for the shortest duration of time.
 * For **very** short durations, prefer busy waiting on a clock.
 *
 * This function is a yield point.
 */
template <class AlarmT, class Rep, class Period>
void sleep_for(AlarmT& alarm, const std::chrono::duration<Rep, Period>& duration);

/**
 * This function yields the control back to the kernel, however,
 * the thread will not be placed back into the runnable queue,
 * which blocks this thread forever in a non-resumable way.
 *
 * The thread will NOT save it's last state, thus it CANNOT be
 * resumed, even with a handle to the thread!
 *
 * If you'd like to implement your own waiting semantics, you should
 * either build on top of `waitable` or use `kern::suspend_self`
 * instead of this.
 *
 * This is a dangerous function and should probably not be used!
 */
[[noreturn]] void block_forever();

/**
 * Causes the current thread to exit, stopping the execution
 * and destroying the stack.
 *
 * No resource will be returned to the OS through an
 * exit. Thus, it must be used with care. Prefer exiting a
 * thread through returning from the entry point function.
 *
 * Behaviour is undefined if this function is called from a
 * non-thread context.
 */
[[noreturn]] void exit(void* res = nullptr);
} // namespace this_thread
} // namespace tos
