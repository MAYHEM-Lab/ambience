//
// Created by fatih on 2/28/19.
//

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace tos
{
    struct stack_size_t
    {
        uint16_t sz;
    };

    template <std::size_t Len>
    struct stack_storage
    {
        using StorageT = std::aligned_storage_t<Len, alignof(std::max_align_t)>;
        StorageT m_storage;
    };
} // namespace tos