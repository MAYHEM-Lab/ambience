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
}
namespace tos {
void swap_context(tos::kern::tcb& current, tos::kern::tcb& to) {
    kern::processor_state context;
    if (save_context(current, context) == return_codes::saved) {
        global::thread_state.current_thread = &to;
        kern::switch_context(to.get_processor_state(), return_codes::scheduled);
    }
}
} // namespace tos
