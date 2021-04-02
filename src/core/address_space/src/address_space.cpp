#include <tos/address_space.hpp>

namespace tos {
address_space::address_space(cur_arch::address_space& space)
    : m_backend(&space) {
    m_backend->space = this;
}

mapping* address_space::containing_mapping(uintptr_t virt_addr) {
    for (auto& mapping : m_mappings) {
        if (contains(mapping.vm_segment.range, virt_addr)) {
            return &mapping;
        }
    }

    return nullptr;
}
} // namespace tos