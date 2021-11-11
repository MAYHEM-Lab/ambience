#include "tos/memory.hpp"
#include <tos/address_space.hpp>
#include <tos/arm/address_space.hpp>

namespace tos::arm {
tos::virtual_range address_space::containing_fragment(const virtual_range& range) const {
    auto mapping = this->containing_mapping(range.base);
    if (!mapping) {
        return {};
    }
    return mapping->vm_segment.range;
}
} // namespace tos::arm