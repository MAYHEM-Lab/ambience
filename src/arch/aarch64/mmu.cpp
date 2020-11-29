#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/mmu.hpp>

namespace tos::aarch64 {
translation_table& get_current_translation_table() {
    return *reinterpret_cast<translation_table*>(get_ttbr0_el1());
}
translation_table& set_current_translation_table(translation_table& table) {
    auto& old = get_current_translation_table();
    set_ttbr0_el1(&table);
    tlb_invalidate_all();
    return old;
}
} // namespace tos::aarch64