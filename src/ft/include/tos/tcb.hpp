#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <tos/arch.hpp>
#include <tos/context.hpp>
#include <tos/interrupt.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/job.hpp>
#include <tos/processor_context.hpp>
#include <tos/utility.hpp>
#include <utility>
#include <tos/fiber/basic_fiber.hpp>

namespace tos::kern {
/**
 * This type represents an execution context in the system.
 *
 * It's expected that a concrete task descriptor type will
 * extend this class to implement required functionality such
 * as starting threads or passing arguments.
 */
struct alignas(alignof(std::max_align_t)) tcb : public job, public basic_fiber<tcb> {
    using job::job;

#define TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
#ifdef TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
    bool log_block_point = false;
#endif

#define TOS_FEATURE_TCB_HAVE_NAME
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    std::string_view name = "<no-name>";
#endif

    tos::list_node<tcb> m_siblings;

public:
    // Called by basic_fiber
    void on_start();
    void on_resume();
    void operator()() override final;
};

inline void set_name(tcb& t, std::string_view name) {
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    t.name = name;
#endif
}

inline std::string_view get_name(const tcb& t) {
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    return t.name;
#else
    return "<names-not-enabled>";
#endif
}
} // namespace tos::kern
namespace tos {
void swap_context(kern::tcb& current, kern::tcb& to, const no_interrupts&);
}
