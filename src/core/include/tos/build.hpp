#pragma once

#include <cstdint>
#include <tos/span.hpp>
#include <string_view>

namespace tos::build {
/**
 * Returns the git commit hash signature when the OS was built.
 * If the commit is unavailable, returns an empty string.
 */
std::string_view commit_hash();

std::string_view arch();

std::string_view platform();

std::string_view drivers();
} // namespace tos::build
