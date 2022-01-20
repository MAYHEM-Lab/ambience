#pragma once

#include <tos/error.hpp>

namespace tos::x86_64 {
enum class mmu_errors
{
    page_alloc_fail,
    already_allocated,
    not_allocated,
    bad_perms,
};

TOS_ERROR_ENUM(mmu_errors);
}