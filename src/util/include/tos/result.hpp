#pragma once

#include <tos/expected.hpp>

namespace tos {
struct common_error {
    template<class T>
    common_error(T&& t);
};

template <class T>
using result = expected<T, common_error>;
}