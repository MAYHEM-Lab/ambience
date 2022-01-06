#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/interrupt_trampoline.hpp>

namespace tos::ae::kernel {
template<class... ServTs, std::size_t... Is>
void do_expose_services_to_group(meta::list<ServTs...>,
                                 std::index_sequence<Is...>,
                                 user_group& group) {
    ((group.channels.push_back(
         std::make_unique<typename ServTs::template async_zerocopy_client<
             tos::ae::downcall_transport>>(make_downcall_factory<ServTs>()(group, Is)))),
     ...);
}

template<class... ServTs>
void expose_services_to_group(meta::list<ServTs...> servs, user_group& group) {
    return do_expose_services_to_group(
        servs, std::make_index_sequence<sizeof...(ServTs)>{}, group);
}

struct in_memory_group {
    template<class GroupDescription, class PlatformArgs>
    static std::unique_ptr<user_group> load(const GroupDescription& desc,
                                            const PlatformArgs& platform_args) {
        auto res = do_load_preemptive_in_memory_group(
            reinterpret_cast<void (*)()>(desc.start_address),
            *platform_args.trampoline,
            desc.name);
        if (res) {
            expose_services_to_group(desc.services, *res);
        }
        return res;
    }

private:
    static std::unique_ptr<user_group> do_load_preemptive_in_memory_group(
        void (*entry)(), interrupt_trampoline& trampoline, std::string_view name);
};
} // namespace tos::ae::kernel