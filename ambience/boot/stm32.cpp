#include <aeboot_generated.hpp>
#include <arch/drivers.hpp>
#include <tos/arm/assembly.hpp>
#include <tos/arm/nvic.hpp>
#include <tos/board.hpp>
#include <tos/ft.hpp>

using bs = tos::bsp::board_spec;

namespace tos::ae::boot {
struct boot_area {
    uint32_t magic;
    int32_t boot_num;
};

[[gnu::section(".nozero")]] volatile boot_area area;

void relocate_vector_table(const arm::vector_table& table, const tos::no_interrupts&) {
    SCB->VTOR = reinterpret_cast<uintptr_t>(&table);
    arm::dsb();
}

expected<void, int> run_image(const boot_record& record) {
    {
        auto uart = bs::default_com::open();
        if (area.magic != 0x12345678) {
            area.magic = 0x12345678;
            area.boot_num = 0;
        } else {
            if (area.boot_num > 3) {
                tos::println(uart, "Detected a bootloop!");
                area.magic = 0;
                return unexpected(-1);
            }
        }

        tos::println(uart, "Welcome to aeboot", area.boot_num);
        area.boot_num = area.boot_num + 1;
    }

    tos::int_guard ig;
    auto ptr = reinterpret_cast<void*>(record.offset());

    // STM32 uses arm images. It starts with the vector table, the first entry is the
    // expected stack pointer and the second entry is the reset handler. We'll switch the
    // stack pointer and jump to the reset handler after relocating the vector table.

    auto vector_table = reinterpret_cast<const arm::vector_table*>(ptr);
    void* sp = ptr;
    auto reset_handler = vector_table->get_handler(arm::nvic_id::reset);

    boot::relocate_vector_table(*vector_table, ig);

    arm::set_stack_ptr(reinterpret_cast<char*>(sp));
    reset_handler();

    TOS_UNREACHABLE();
}

const boot_record* select_image() {
    static constexpr auto image = boot_record{0x8008000, false};
    return &image;
}

expected<void, int> boot() {
    auto image = select_image();
    if (!image) {
        // error
    }

    return run_image(*image);
}
} // namespace tos::ae::boot

tos::stack_storage store;
void tos_main() {
    tos::launch(store, tos::ae::boot::boot);
}