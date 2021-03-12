#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/arm/cmsis.hpp>
#include <tos/span.hpp>

namespace tos::arm {
constexpr bool memmanage_exception_supported() {
#if defined(SCB_SHCSR_MEMFAULTENA_Msk)
    return true;
#endif
    return false;
}

/**
 * Returns the number of supported interrupts on this processor.
 *
 * The number is not exact since the supported number of interrupts
 * are always multiplies of 32. For instance, if the processor actually
 * has 33 interrupts, the function will return 64.
 *
 * The information is gathered from SCnSCB->ICTR.
 */
int number_of_supported_interrupts();

struct exception_id_t {
    int8_t m_id;
};

namespace nvic_id {
static constexpr exception_id_t reset{-15};
static constexpr exception_id_t nmi{-14};
static constexpr exception_id_t hard_fault{-13};
static constexpr exception_id_t memory_fault{-12};
static constexpr exception_id_t bus_fault{-11};
static constexpr exception_id_t usage_fault{-10};
// Between svcall and usage_fault are reserved
static constexpr exception_id_t svcall{-5};
static constexpr exception_id_t debug{-4};
static constexpr exception_id_t pendsv{-2};
static constexpr exception_id_t systick{-1};
} // namespace nvic_id

using nvic_raw_handler_t = void (*)();

struct [[gnu::packed]] vector_table {
    /**
     * Gets the current raw handler for the given exception.
     */
    [[nodiscard]] nvic_raw_handler_t get_handler(exception_id_t) const;

    [[nodiscard]] span<const nvic_raw_handler_t> handlers() const;

    [[nodiscard]] span<nvic_raw_handler_t> handlers();

    void* stack_ptr;
    union {
        nvic_raw_handler_t vectors[15];
        struct {
            nvic_raw_handler_t reset;
            nvic_raw_handler_t nmi;
            nvic_raw_handler_t hard_fault;
            nvic_raw_handler_t memory_fault;
            nvic_raw_handler_t bus_fault;
            nvic_raw_handler_t usage_fault;
            nvic_raw_handler_t __reserved__[4];
            nvic_raw_handler_t svcall;
            nvic_raw_handler_t debug;
            nvic_raw_handler_t __reserved2__[1];
            nvic_raw_handler_t pendsv;
            nvic_raw_handler_t systick;
        };
    };
};
static_assert(offsetof(vector_table, stack_ptr) == 0);
static_assert(offsetof(vector_table, vectors) == sizeof(void*));

/**
 * Returns a reference to the current vector table.
 *
 * The address of the vector table is acquired from SCB->VTOR, therefore
 * always returns the correct table. If the table is changed, the returned
 * table will be invalid.
 */
vector_table& get_vector_table();

/**
 * This driver implements a dynamic vector table for ARMv7m Nested Vector
 * Interrupt Controllers.
 *
 * The dynamic piece means that interrupt handlers can be registered at
 * runtime as opposed to having to be defined at build time.
 *
 * This is achieved by relocating the vector table to RAM.
 *
 * Currently, TOS does not support nested interrupts, therefore the interrupt
 * priority feature of the NVIC is not exposed and all interrupts will have
 * the same priority.
 */
struct alignas(128) dynamic_vector_table : vector_table {
public:
    /**
     * Sets the raw handler for the given exception.
     */
    void set_handler(exception_id_t, nvic_raw_handler_t);

private:
    dynamic_vector_table() = default;

    friend dynamic_vector_table& make_dynamic_vector_table(span<uint8_t> buffer);
};

static_assert(offsetof(dynamic_vector_table, stack_ptr) == 0);

dynamic_vector_table& make_dynamic_vector_table(span<uint8_t> buffer);

void relocate_vector_table(const vector_table& table);
} // namespace tos::arm