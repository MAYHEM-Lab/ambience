#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos::ae::kernel {
template<class... ServTs, std::size_t... Is>
void do_expose_services_to_group(meta::list<ServTs...>,
                                 std::index_sequence<Is...>,
                                 user_group& group) {
    ((group.channels.push_back(
         std::make_unique<typename ServTs::template async_zerocopy_client<
             tos::ae::downcall_transport>>(meta::id<ServTs>{}, group, Is))),
     ...);
}

template<class... ServTs>
void expose_services_to_group(meta::list<ServTs...> servs, user_group& group) {
    return do_expose_services_to_group(
        servs, std::make_index_sequence<sizeof...(ServTs)>{}, group);
}

struct preemptive_elf_group {
    template<class GroupDescription, class PlatformArguments>
    static std::unique_ptr<user_group> load(const GroupDescription& info,
                                            const PlatformArguments& platform_args) {
        auto res = do_load(info.elf_body,
                           *platform_args.trampoline,
                           *platform_args.page_alloc,
                           *platform_args.table,
                           info.name);
        if (res) {
            expose_services_to_group(info.services, *res);
        }
        return res;
    }

private:
    static std::unique_ptr<user_group> do_load(span<const uint8_t> elf_body,
                                               interrupt_trampoline& trampoline,
                                               physical_page_allocator& palloc,
                                               cur_arch::translation_table& root_table,
                                               std::string_view name);
};
} // namespace tos::ae::kernel