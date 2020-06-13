#pragma once

#include <tos/x86/spmanip.hpp>

namespace tos::platform {
inline void enable_interrupts() {
}
inline void disable_interrupts() {
}
namespace arch = x86;
} // namespace tos::platform

#include <boost/asio/io_service.hpp>
boost::asio::io_service& get_io();