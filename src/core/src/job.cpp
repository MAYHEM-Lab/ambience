#include <tos/job.hpp>

namespace tos {
job::job(context& ctx) : m_context{&ctx} {
}

void job::set_context(context& ctx) {
    on_set_context(ctx);
    m_context = &ctx;
}

context& job::get_context() {
    return *m_context;
}
}