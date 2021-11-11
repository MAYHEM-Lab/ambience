#pragma once

#include <type_traits>

namespace tos {
template<class T, class U>
concept IsSame = std::is_same<T, U>::value;
}