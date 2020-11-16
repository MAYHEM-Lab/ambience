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
    case return_codes::saved:
        global::thread_state.current_thread = this;
        switch_context(*m_ctx, return_codes::scheduled);
    case return_codes::yield:
    case return_codes::suspend:
        break;
    case return_codes::do_exit:
        // TODO(#34): Potentially a use-after-free. See the issue.
        std::destroy_at(this);
        break;
    case return_codes::scheduled:
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

    switch_context(global::thread_state.backup_state, return_codes::do_exit);
}

void suspend_self(const no_interrupts&) {
    kern::processor_state ctx;
    if (save_context(*self(), ctx) == return_codes::saved) {
        switch_context(global::thread_state.backup_state, return_codes::suspend);
    }
}
} // namespace tos::kern

namespace tos {
void swap_context(tos::kern::tcb& current, tos::kern::tcb& to, const no_interrupts&) {
    kern::processor_state context;
    if (save_context(current, context) == return_codes::saved) {
        global::thread_state.current_thread = &to;
        kern::switch_context(to.get_processor_state(), return_codes::scheduled);
    }
}
} // namespace tos

namespace tos::this_thread {
void block_forever() {
    kern::disable_interrupts();
    switch_context(global::thread_state.backup_state, return_codes::suspend);
}

void yield() {
    tos::int_guard ig;
    kern::processor_state ctx;
    if (save_context(*self(), ctx) == return_codes::saved) {
        kern::make_runnable(*self());
        switch_context(global::thread_state.backup_state, return_codes::yield);
    }
}
} // namespace tos::this_thread