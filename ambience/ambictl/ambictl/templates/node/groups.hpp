#pragma once

#include <tuple>

// clang-format off
{% for include in group_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

using groups_t = std::tuple<{{groups | join(", ")}}>;

groups_t init_all_groups(tos::interrupt_trampoline& trampoline,
                         tos::physical_page_allocator& palloc,
                         tos::cur_arch::translation_table& root_table);