#pragma once

#include "component.hpp"

#include <tos/intrusive_list.hpp>

namespace tos {
namespace kern {
class tcb;
}

struct threads_component : id_component<3> {
    void thread_created(kern::tcb&);
    void thread_exited(kern::tcb&);

    intrusive_list<kern::tcb> threads;
};
} // namespace tos