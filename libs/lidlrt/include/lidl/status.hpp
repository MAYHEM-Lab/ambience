#pragma once

#include <optional>

namespace lidl {
template<class T>
using status = std::optional<T>;

template<class ResT, class ErrT>
struct expected {
    expected(ResT p_res)
        : res{p_res}
        , has_res(true) {
    }

    union {
        ResT res;
        ErrT err;
    };
    bool has_res;
};
} // namespace lidl