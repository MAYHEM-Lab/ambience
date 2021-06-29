#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/interrupt_trampoline.hpp>

namespace tos::ae::kernel {
template<class... ServTs>
void expose_services_to_group(meta::list<ServTs...>, user_group& group) {
    ((group.channels.push_back(
         std::make_unique<typename ServTs::template async_zerocopy_client<
             tos::ae::downcall_transport>>(*group.iface.user_iface, 0))),
     ...);
}

struct in_memory_group {
    template<class GroupDescription, class PlatformArgs>
    static std::unique_ptr<user_group> load(const GroupDescription& desc,
                                            const PlatformArgs& platform_args) {
        auto res = do_load_preemptive_in_memory_group(
            reinterpret_cast<void (*)()>(desc.start_address), *platform_args.trampoline);
        if (res) {
            expose_services_to_group(desc.services, *res);
        }
        return res;
    }

private:
    static std::unique_ptr<user_group>
    do_load_preemptive_in_memory_group(void (*entry)(), interrupt_trampoline& trampoline);
};
} // namespace tos::ae::kernel