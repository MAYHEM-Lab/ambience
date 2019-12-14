//
// Created by fatih on 10/14/19.
//

#pragma once

#include <array>
#include <tos/arch.hpp>
#include <tos/span.hpp>
#include <tos/thread.hpp>

namespace tos {
namespace debug {
template<class LogT>
void NO_INLINE dump_stack(LogT& log) {
    auto stack_top = tos::this_thread::get_id().id;
    auto cur_stack = reinterpret_cast<uintptr_t>(tos_get_stack_ptr());
    auto cur_ptr = reinterpret_cast<uint8_t*>(cur_stack);
    auto size = stack_top - cur_stack;
    auto stack_span = tos::span<const uint8_t>(cur_ptr, size);
    static constexpr std::array<uint8_t, 8> separator{
        '$', 't', 'o', 's', 's', '$', '#', '\n'};
    log->write(separator);
    log->write(tos::raw_cast<const uint8_t>(tos::monospan(size)));
    log->write(stack_span);
}
} // namespace debug
} // namespace tos