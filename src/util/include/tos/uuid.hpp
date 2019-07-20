//
// Created by fatih on 6/22/19.
//

#pragma once

#include <cstdint>
#include <utility>
#include <algorithm>
#include <cstring>

namespace tos
{
struct uuid {
    uint8_t id_[16];
};

inline bool operator==(const uuid& a, const uuid& b) {
    return std::equal(std::begin(a.id_), std::end(a.id_), std::begin(b.id_));
}

inline bool operator!=(const uuid& a, const uuid& b) {
    return !(a==b);
}
}
