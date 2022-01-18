#pragma once

#include <cstddef>
#include <string_view>
#include <tos/expected.hpp>
#include <type_traits>
#include <tos/error.hpp>

namespace tos {
template<class T>
using result = expected<T, any_error>;
} // namespace tos