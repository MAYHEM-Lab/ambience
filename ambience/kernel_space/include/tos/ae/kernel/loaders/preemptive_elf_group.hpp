#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos::ae::kernel {
std::unique_ptr<user_group>
do_load_preemptive_elf_group(span<const uint8_t> elf_body,
                             interrupt_trampoline& trampoline,
                             physical_page_allocator& palloc,
                             cur_arch::translation_table& root_table);

template<class... ServTs>
void expose_services_to_group(meta::list<ServTs...>, user_group& group) {
    ((group.channels.push_back(
        std::make_unique<typename ServTs::template async_zerocopy_client<
            tos::ae::downcall_transport>>(*group.iface.user_iface, 0))),
        ...);
}

template<class GroupDescription>
std::unique_ptr<user_group>
load_preemptive_elf_group(const GroupDescription& info,
                          interrupt_trampoline& trampoline,
                          physical_page_allocator& palloc,
                          cur_arch::translation_table& root_table) {
    auto res =
        do_load_preemptive_elf_group(info.elf_body, trampoline, palloc, root_table);
    if (res) {
        expose_services_to_group(info.services, *res);
    }
    return res;
}
}