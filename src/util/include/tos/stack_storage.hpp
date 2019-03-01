//
// Created by fatih on 2/28/19.
//

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace tos
{
    template <std::size_t Len>
    using stack_storage = std::aligned_storage_t<Len, alignof(std::max_align_t)>;
} // namespace tos