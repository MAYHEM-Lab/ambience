#pragma once

#include <tos/x86/spmanip.hpp>

namespace tos::platform {
inline bool interrupts_disabled() {
    return true;
}
inline void enable_interrupts() {
}
inline void disable_interrupts() {
}

[[noreturn]]
void force_reset();
} // namespace tos::platform

#include <boost/asio/io_service.hpp>
boost::asio::io_service& get_io();