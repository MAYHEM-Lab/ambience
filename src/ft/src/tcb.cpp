#include <tos/tcb.hpp>
#include <tos/components/threads.hpp>

namespace tos::kern {
void tcb::set_context(context& ctx) {
    if (auto threads = m_context->get_component<threads_component>(); threads) {
        threads->thread_dissociated(*this);
    }
    m_context = &ctx;
    if (auto threads = m_context->get_component<threads_component>(); threads) {
        threads->thread_adopted(*this);
    }
}

context& tcb::get_context() {
    return *m_context;
}

tcb::tcb(context& ctx_ptr)
    : m_context{&ctx_ptr} {
    if (auto threads = m_context->get_component<threads_component>(); threads) {
        threads->thread_created(*this);
    }
}

tcb::~tcb() {
    if (auto threads = m_context->get_component<threads_component>(); threads) {
        threads->thread_exited(*this);
    }
}
}