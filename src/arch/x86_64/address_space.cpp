#include <tos/x86_64/address_space.hpp>
#include <tos/address_space.hpp>
#include <tos/backing_object.hpp>

namespace tos::x86_64 {
expected<bool, mmu_errors>
address_space::handle_memory_fault(const exception_frame& frame, uintptr_t fault_addr) {
    auto mapping = containing_mapping(fault_addr);

    if (!mapping) {
        return false;
    }

    memory_fault fault;
    fault.map = mapping;
    fault.virt_addr = fault_addr;

    if (!mapping->obj->handle_memory_fault(fault)) {
        return false;
    }

    invlpg(fault_addr);

    return true;
}
} // namespace tos::x86_64