#pragma once

#include <tos/x86_64/port.hpp>

namespace tos::x86_64 {
enum pic_cmd_codes : uint8_t
{
    icw4 = 0x1,
    init = 0x10,
    end_of_interrupt = 0x20
};

inline void io_wait() {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

class pic {
public:
    static void initialize() {
        // init with ICW4 needed (0x10 = init, 0x01 = read icw4)
        master_cmd().outb(0x11);
        io_wait();
        slave_cmd().outb(0x11);
        io_wait();

        // ICW2
        master_data().outb(0x20); // IRQ offset at 32
        io_wait();
        slave_data().outb(0x28); // IRQ offset at 40
        io_wait();

        // ICW3
        master_data().outb(0x04); // there is a slave at IRQ2 (0100)
        io_wait();
        slave_data().outb(0x02); // slave id is 2
        io_wait();

        // ICW4 = we're in 8086 mode
        master_data().outb(0x01);
        io_wait();
        slave_data().outb(0x01);
        io_wait();

        // Masks, all disabled
        master_data().outb(0xff);
        slave_data().outb(0xff);
        enable_irq(2);
    }

    static void enable_irq(int irq) {
        if (irq >= 8) {
            slave_data().outb(slave_data().inb() & ~(1 << (irq - 8)));
            return;
        }
        master_data().outb(master_data().inb() & ~(1 << (irq)));
    }

    static void disable_irq(int irq) {
        if (irq >= 8) {
            slave_data().outb(slave_data().inb() | 1 << (irq - 8));
            return;
        }
        master_data().outb(master_data().inb() | 1 << (irq));
    }

private:
    static port master_cmd() {
        return {0x20};
    }

    static port master_data() {
        return {0x21};
    }

    static port slave_cmd() {
        return {0xA0};
    }

    static port slave_data() {
        return {0xA1};
    }
};
} // namespace tos::x86_64