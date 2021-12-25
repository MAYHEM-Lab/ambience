#include "tos/fiber/basic_fiber.hpp"
#include <tos/components/threads.hpp>
#include <tos/ft.hpp>
#include <tos/tcb.hpp>

namespace tos::kern {
tcb::tcb(context& ctx)
    : job(ctx) {
    if (auto threads = get_context().get_component<threads_component>(); threads) {
        threads->thread_created(*this);
    }
}

tcb::~tcb() {
    if (auto threads = get_context().get_component<threads_component>(); threads) {
        threads->thread_exited(*this);
    }
}

void tcb::operator()() {
    this->resume([this] { global::thread_state.current_thread = this; });
    global::thread_state.current_thread = nullptr;
}

void suspend_self(const no_interrupts&) {
#ifdef TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
    if (self()->log_block_point) {
        LOG_TRACE("Blocked at", (void*)__builtin_return_address(0));
    }
#endif

    self()->suspend();
}

thread_id_t start(tcb& t, void (*entry)()) {
    auto ctx_ptr = new ((char*)&t - sizeof(processor_context)) processor_context;
    start(*ctx_ptr, entry, &t);

    t.set_processor_state(*ctx_ptr);

    make_runnable(t);
    global::thread_state.num_threads++;
    return {reinterpret_cast<uintptr_t>(static_cast<tcb*>(&t))};
}
} // namespace tos::kern

namespace tos {
void swap_context(kern::tcb& current, kern::tcb& to, const no_interrupts&) {
    swap_fibers(current, to, [&to] { global::thread_state.current_thread = &to; });
}
} // namespace tos

namespace tos::this_thread {
void yield(const no_interrupts&) {
    self()->suspend([] { kern::make_runnable(*self()); });
}

void block_forever() {
    kern::disable_interrupts();
    self()->suspend_final(context_codes::suspend);
}

void exit(void*) {
    kern::disable_interrupts();
    self()->suspend_final(context_codes::do_exit);
}
} // namespace tos::this_thread