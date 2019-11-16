//
// Created by fatih on 2/28/19.
//

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <tos/span.hpp>

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
        operator span<uint8_t>() {
            return tos::raw_cast<uint8_t>(tos::monospan(m_storage));
        }
    };
} // namespace tos