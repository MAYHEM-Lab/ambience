#pragma once

#include <tos/components/component.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/tcb.hpp>

namespace tos {
struct threads_component : id_component<3> {
    void thread_created(kern::tcb&);
    void thread_adopted(kern::tcb&);
    void thread_dissociated(kern::tcb&);
    void thread_exited(kern::tcb&);

    intrusive_list<kern::tcb, through_member<&kern::tcb::m_siblings>> threads;
};
} // namespace tos