#include <tos/address_space.hpp>
#include <tos/arm/address_space.hpp>

namespace tos::arm {
tos::memory_range address_space::containing_fragment(const memory_range& range) const {
    auto mapping = space->containing_mapping(range.base);
    if (!mapping) {
        return {};
    }
    return mapping->vm_segment.range;
}
} // namespace tos::arm