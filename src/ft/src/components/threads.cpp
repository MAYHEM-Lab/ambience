#include <tos/components/threads.hpp>

namespace tos {
void threads_component::thread_created(kern::tcb& thread) {
    threads.push_back(thread);
}

void threads_component::thread_adopted(kern::tcb& thread) {
    threads.push_back(thread);
}

void threads_component::thread_dissociated(kern::tcb& thread) {
    threads.erase(threads.unsafe_find(thread));
}

void threads_component::thread_exited(kern::tcb& thread) {
    threads.erase(threads.unsafe_find(thread));
}
} // namespace tos