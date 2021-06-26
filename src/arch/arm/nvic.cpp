#include "tos/arm/assembly.hpp"
#include "tos/debug/assert.hpp"
#include <tos/arm/cmsis.hpp>
#include <tos/arm/nvic.hpp>
#include <tos/interrupt.hpp>

namespace tos::arm {
vector_table& get_vector_table() {
    auto ptr = reinterpret_cast<vector_table*>(SCB->VTOR);
    return *ptr;
}

nvic_raw_handler_t vector_table::get_handler(exception_id_t id) const {
    return *(vectors + id.m_id + 15);
}

void dynamic_vector_table::set_handler(exception_id_t id, nvic_raw_handler_t handler) {
    *(vectors + id.m_id + 15) = handler;
}

dynamic_vector_table& make_dynamic_vector_table(span<uint8_t> buffer) {
    // Vector tables must be at least 128 byte aligned
    Assert((reinterpret_cast<uintptr_t>(buffer.data()) & 0x7F) == 0);
    return *new (buffer.data()) dynamic_vector_table;
}

void relocate_vector_table(const vector_table& table) {
    tos::int_guard ig;
    SCB->VTOR = reinterpret_cast<uintptr_t>(&table);
    dsb();
}

int number_of_supported_interrupts() {
#if __CORTEX_M == 0
    return 64;
#else
    return (SCnSCB->ICTR + 1) * 32;
#endif
}

span<const nvic_raw_handler_t> vector_table::handlers() const {
    return span<const nvic_raw_handler_t>{&vectors[0],
                                          size_t(number_of_supported_interrupts())};
}

span<nvic_raw_handler_t> vector_table::handlers() {
    return span<nvic_raw_handler_t>{&vectors[0],
                                    size_t(number_of_supported_interrupts())};
}
} // namespace tos::arm
