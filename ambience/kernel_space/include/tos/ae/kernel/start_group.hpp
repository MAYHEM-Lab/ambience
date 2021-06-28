#pragma once

#include <memory>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/span.hpp>

namespace tos::ae::kernel {
std::unique_ptr<user_group> start_group(tos::span<uint8_t> stack,
                                        void (*entry)(),
                                        tos::interrupt_trampoline& trampoline);
}