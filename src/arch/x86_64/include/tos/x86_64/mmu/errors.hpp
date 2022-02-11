#pragma once

#include <tos/error.hpp>

namespace tos::x86_64 {
enum class mmu_errors
{
    no_allocator,
    page_alloc_fail,
    already_allocated,
    not_allocated,
    bad_perms,
    bad_alignment,
};

TOS_ERROR_ENUM(mmu_errors);
}