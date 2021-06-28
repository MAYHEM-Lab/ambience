#include <tos/ae/kernel/loaders/in_memory_user.hpp>
#include <tos/ae/kernel/start_group.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/interrupt_trampoline.hpp>

namespace tos::ae::kernel {
std::unique_ptr<user_group>
do_load_preemptive_in_memory_group(void (*entry)(), interrupt_trampoline& trampoline) {
    auto stack = new char[TOS_DEFAULT_STACK_SIZE];

    return start_group(
        {reinterpret_cast<uint8_t*>(stack), TOS_DEFAULT_STACK_SIZE}, entry, trampoline);
}
} // namespace tos::ae::kernel