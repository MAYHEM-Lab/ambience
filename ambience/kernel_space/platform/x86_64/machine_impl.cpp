#include <lai/helpers/pm.h>
#include <machine_generated.hpp>
#include <tos/ae/registry.hpp>
#include <tos/x86_64/port.hpp>
#include <tos/x86_64/assembly.hpp>

namespace {
[[noreturn]]
void reboot() {
    uint8_t good = 0x02;
    while (good & 0x02) good = tos::x86_64::port(0x64).inb();
    tos::x86_64::port(0x64).outb(0xFE);
    tos::x86_64::hlt();
    while (true);
}
struct machine_impl : tos::ae::services::machine::sync_server {
    bool reboot() override {
        if (LAI_ERROR_NONE == lai_acpi_reset()) {
            return true;
        }
        ::reboot();
    }
};
} // namespace

tos::ae::registry_base& get_registry();
void do_machine_impl() {
    get_registry().register_service("machine", new machine_impl);
}