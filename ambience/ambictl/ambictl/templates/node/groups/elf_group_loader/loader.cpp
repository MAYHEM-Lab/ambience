#include <registry.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/arch.hpp>
#include <tos/elf.hpp>
#include <tos/expected.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <{{group_name}}.hpp>

// clang-format off
#include <{{group_name}}_elf.hpp>
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

tos::expected<std::unique_ptr<tos::ae::kernel::user_group>, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table);

auto init_{{group_name}}(tos::interrupt_trampoline& trampoline,
                         tos::physical_page_allocator& palloc,
                         tos::cur_arch::translation_table& root_table)
    -> tos::expected<std::unique_ptr<tos::ae::kernel::user_group>, int> {
    // clang-format off
    LOG({{group_name}}_elf.slice(0, 4));
    auto elf_res = tos::elf::elf64::from_buffer({{group_name}}_elf);
    // clang-format on
    if (!elf_res) {
        LOG_ERROR("Could not parse payload!");
        LOG_ERROR("Error code: ", int(force_error(elf_res)));
        return tos::unexpected(42);
    }

    auto load_res = load_from_elf(force_get(elf_res), trampoline, palloc, root_table);
    auto& group = force_get(load_res);

    {% for service_name in services %}
    group->channels.push_back(
        std::make_unique<{{services[service_name]}}::service_type::async_zerocopy_client<
            tos::ae::downcall_transport>>(*group->iface.user_iface, {{loop.index - 1}}));
    {% endfor %}

    return std::move(group);
}