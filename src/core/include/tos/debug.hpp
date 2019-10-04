//
// Created by fatih on 7/12/18.
//

#pragma once

#include <array>
#include <tos/arch.hpp>
#include <tos/span.hpp>
#include <tos/thread.hpp>

namespace tos {
namespace kern {
/**
 * This function is used to signal a non-recoverable fault in
 * the system.
 *
 * The execution of the whole operating system will be suspended.
 *
 * In a debug environment, the OS will idle forever.
 *
 * In a release environment, if possible, the incident will be
 * logged and the system will be rebooted.
 *
 * Should be used only for fatal errors such as broken invariants.
 *
 * @tparam ErrT an explanation for the crash
 */
template<class ErrT>
[[noreturn]] void fatal(ErrT&&) noexcept {
    // or directly reset?
    tos_force_reset();
    // tos::this_thread::block_forever();
}
} // namespace kern

namespace debug {
template<class ErrorTagType>
void panic(ErrorTagType&& error_tag) {
    kern::fatal(error_tag);
}

template<class LogT>
void NO_INLINE dump_stack(LogT& log) {
    auto stack_top = tos::this_thread::get_id().id;
    auto cur_stack = reinterpret_cast<uintptr_t>(read_sp());
    auto cur_ptr = reinterpret_cast<char*>(cur_stack);
    auto size = stack_top - cur_stack;
    auto stack_span = tos::span<const char>(cur_ptr, size);
    static constexpr std::array<char, 8> separator{
        '$', 't', 'o', 's', 's', '$', '#', '\n'};
    log->write(separator);
    log->write(tos::raw_cast<const char>(tos::monospan(size)));
    log->write(stack_span);
}
} // namespace debug
} // namespace tos

#if defined(TOS_ARCH_esp8266)
extern "C" int ets_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
#define tos_debug_print
#endif

#if defined(TOS_ARCH_nrf52)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_stm32)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_x86)
#include <stdio.h>
#define tos_debug_print printf
#endif

#if defined(TOS_ARCH_atmega)
#define tos_debug_print(...)
#endif

#if !defined(tos_debug_print)
#endif

#define Expects(x)
#define Ensures(x)