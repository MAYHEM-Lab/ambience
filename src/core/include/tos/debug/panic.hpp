//
// Created by fatih on 10/14/19.
//

#pragma once

#include <tos/debug/log.hpp>

namespace tos {
namespace platform {
[[noreturn]] void force_reset();
}
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
 */
[[noreturn]] void panic(const char* err);
} // namespace debug
} // namespace tos
