#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/interrupt_trampoline.hpp>

namespace tos::ae::kernel {
std::unique_ptr<user_group>
do_load_preemptive_in_memory_group(void (*entry)(), interrupt_trampoline& trampoline);

template<class GroupDescription>
std::unique_ptr<user_group>
load_preemptive_in_memory_group(const GroupDescription& desc,
                                interrupt_trampoline& trampoline);
} // namespace tos::ae::kernel