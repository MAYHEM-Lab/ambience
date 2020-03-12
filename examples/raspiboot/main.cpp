#include <arch/drivers.hpp>
#include <asm.hpp>
#include <tos/debug/log.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>


extern "C" {
void tos_force_reset() {
    tos::aarch64::intrin::bkpt();
}
}

tos::raspi3::system_timer timer;

extern "C" {
[[gnu::used]] void
exc_handler(uint32_t type, uint32_t esr, uint32_t elr, uint32_t spsr, uint32_t far) {
    // print out interruption type
    switch (type) {
    case 0:
        LOG("Synchronous");
        break;
    case 1:
        LOG("IRQ");
        break;
    case 2:
        LOG("FIQ");
        break;
    case 3:
        LOG("SError");
        break;
    }
    if (type == 1) {
        timer.irq();
        return;
    }
    LOG(": ");
    // decode exception type (some, not all. See ARM DDI0487B_b chapter D10.2.28)
    switch (esr >> 26) {
    case 0b000000:
        LOG("Unknown");
        break;
    case 0b000001:
        LOG("Trapped WFI/WFE");
        break;
    case 0b001110:
        LOG("Illegal execution");
        break;
    case 0b010101:
        LOG("System call");
        break;
    case 0b100000:
        LOG("Instruction abort, lower EL");
        break;
    case 0b100001:
        LOG("Instruction abort, same EL");
        break;
    case 0b100010:
        LOG("Instruction alignment fault");
        break;
    case 0b100100:
        LOG("Data abort, lower EL");
        break;
    case 0b100101:
        LOG("Data abort, same EL");
        break;
    case 0b100110:
        LOG("Stack alignment fault");
        break;
    case 0b101100:
        LOG("Floating point");
        break;
    default:
        LOG("Unknown");
        break;
    }
    // decode data abort cause
    if (esr >> 26 == 0b100100 || esr >> 26 == 0b100101) {
        LOG(", ");
        switch ((esr >> 2) & 0x3) {
        case 0:
            LOG("Address size fault");
            break;
        case 1:
            LOG("Translation fault");
            break;
        case 2:
            LOG("Access flag fault");
            break;
        case 3:
            LOG("Permission fault");
            break;
        }
        switch (esr & 0x3) {
        case 0:
            LOG(" at level 0");
            break;
        case 1:
            LOG(" at level 1");
            break;
        case 2:
            LOG(" at level 2");
            break;
        case 3:
            LOG(" at level 3");
            break;
        }
    }
    // dump registers
    LOG(":\n  ESR_EL1 ");
    LOG(esr >> 32);
    LOG(esr);
    LOG(" ELR_EL1 ");
    LOG(elr >> 32);
    LOG(elr);
    LOG("\n SPSR_EL1 ");
    LOG(spsr >> 32);
    LOG(spsr);
    LOG(" FAR_EL1 ");
    LOG(far >> 32);
    LOG(far);
    LOG("\n");
}
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE> stack;
void tos_main() {
    tos::launch(stack, [] {
        tos::raspi3::uart0 uart;
        tos::println(uart, "Hello from tos!");

        uint32_t el;
        asm volatile("mrs %0, CurrentEL" : "=r"(el));
        tos::println(uart, "Execution Level:", int((el >> 2) & 3));

        tos::raspi3::framebuffer fb({1024, 768});
        tos::println(uart, fb.dims().width, fb.dims().height);
        tos::println(uart, fb.get_buffer().data(), fb.get_buffer().size_bytes());

        tos::println(uart,
                     tos::itoa(reinterpret_cast<uint64_t>(
                                   &bcm2837::INTERRUPT_CONTROLLER->enable_irq_1),
                               16));

        auto fn = [] {

        };

        timer.set_callback(tos::function_ref<void()>(fn));
        timer.enable();

        int dir = 1;
        while (true) {
            for (auto& c : fb.get_buffer()) {
                c += dir;
            }
            if (fb.get_buffer()[0] == 255 || fb.get_buffer()[0] == 0) {
                dir *= -1;
            }
            tos::println(
                uart, bcm2837::SYSTEM_TIMER->counter_lo, bcm2837::SYSTEM_TIMER->compare1);
        }

        tos::println(uart, "Filled");

        while (true) {
            uint8_t c;
            uart->write(uart->read(tos::monospan(c)));
        }
    });
}