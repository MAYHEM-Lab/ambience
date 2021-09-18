#include <tos/address_space.hpp>
#include <tos/arch.hpp>
#include <type_traits>

namespace tos {
static_assert(std::is_base_of_v<address_space, cur_arch::address_space>);
cur_arch::address_space* address_space::self() {
    return static_cast<cur_arch::address_space*>(this);
}

const cur_arch::address_space* address_space::self() const {
    return static_cast<const cur_arch::address_space*>(this);
}

mapping* address_space::containing_mapping(uintptr_t virt_addr) {
    for (auto& mapping : m_mappings) {
        if (contains(mapping.vm_segment.range, virt_addr)) {
            return &mapping;
        }
    }

    return nullptr;
}

void address_space::add_mapping(mapping& mapping) {
    m_mappings.push_back(mapping);
    mapping.va = self();
}

const auto& address_space::mappings() const {
    return m_mappings;
}

namespace global {
cur_arch::address_space* cur_as;
}
} // namespace tos