//
// Created by fatih on 10/14/19.
//

#pragma once

#include <tos/platform.hpp>
#include <tos/debug/log.hpp>

namespace tos {
namespace debug {
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
 * @tparam ErrorTagType type for an explanation for the crash
 */
TOS_NO_OPTIMIZE
[[noreturn]] inline void panic(const char* err) {
    tos::debug::fatal(err);
    tos::platform::force_reset();
}
} // namespace debug
} // namespace tos
