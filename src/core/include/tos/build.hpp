#pragma once

#include <cstdint>
#include <tos/span.hpp>

namespace tos::build {
/**
 * Returns the git commit hash signature when the OS was built.
 * If the commit is unavailable, returns an empty span.
 */
span<const uint8_t> commit_hash();
} // namespace tos::build
