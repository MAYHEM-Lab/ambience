#pragma once

#include <cstdint>
#include <string_view>
#include <tos/task.hpp>
#include <tos/ae/rings.hpp>
#include <tos/flags.hpp>

namespace tos::ae {
void proc_res_queue(interface& iface);

tos::Task<void> log_str(std::string_view sv);
} // namespace tos::ae