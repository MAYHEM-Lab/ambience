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
    auto save_res = save_ctx(global::thread_state.backup_state);

    switch (save_res) {
    case context_codes::saved:
        global::thread_state.current_thread = this;
        switch_context(*m_ctx, context_codes::scheduled);
    case context_codes::yield:
    case context_codes::suspend:
        break;
    case context_codes::do_exit:
        std::destroy_at(this);
        break;
    case context_codes::scheduled:
        tos::debug::panic("Should not happen");
    }

    global::thread_state.current_thread = nullptr;
}

void tcb::on_set_context(context& new_ctx) {
    if (auto threads = get_context().get_component<threads_component>(); threads) {
        threads->thread_dissociated(*this);
    }

    if (auto threads = new_ctx.get_component<threads_component>(); threads) {
        threads->thread_adopted(*this);
    }
}

void thread_exit() {
    kern::disable_interrupts();

    // no need to save the current context, we'll exit

    switch_context(global::thread_state.backup_state, context_codes::do_exit);
}

void suspend_self(const no_interrupts&) {
#ifdef TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
    if (self()->log_block_point) {
        LOG_TRACE("Blocked at", (void*)__builtin_return_address(0));
    }
#endif

    processor_context ctx;
    if (save_context(*self(), ctx) == context_codes::saved) {
        switch_context(global::thread_state.backup_state, context_codes::suspend);
    }
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
    processor_context context;

    current.set_processor_state(context);
    global::thread_state.current_thread = &to;
    swap_context(context, to.get_processor_state());
}
} // namespace tos

namespace tos::this_thread {
void block_forever() {
    kern::disable_interrupts();
    switch_context(global::thread_state.backup_state, context_codes::suspend);
}

void yield(const no_interrupts&) {
    processor_context ctx;
    if (save_context(*self(), ctx) == context_codes::saved) {
        kern::make_runnable(*self());
        switch_context(global::thread_state.backup_state, context_codes::yield);
    }
}
} // namespace tos::this_thread