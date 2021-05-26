#include "groups.hpp"
#include "registry.hpp"
#include <tos/detail/poll.hpp>

groups_t init_all_groups(tos::interrupt_trampoline& trampoline,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::translation_table& root_table) {
    // clang-format off
{% for group in groups %}
    ::{{group}} {{group}}{force_get(init_{{group}}(trampoline, palloc, root_table))};
    tos::coro::make_detached({{group}}.init_dependencies(registry));
{% endfor %}

    // TODO: add std::move to these somehow
    return { {{groups|join(", ")}} };
    // clang-format on
}
