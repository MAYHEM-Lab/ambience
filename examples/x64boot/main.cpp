#include "tos/function_ref.hpp"
#include "tos/memory.hpp"
#include "tos/paging/physical_page_allocator.hpp"
#include "tos/self_pointing.hpp"
#include "tos/semaphore.hpp"
#include "tos/x86_64/exception.hpp"
#include "tos/x86_64/port.hpp"
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/virtio.hpp>
#include <tos/virtio/block_device.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/x86_64/pic.hpp>

void dump_table(tos::cur_arch::translation_table& table) {
    tos::cur_arch::traverse_table_entries(
        table, [](tos::memory_range range, tos::cur_arch::table_entry& entry) {
            LOG_TRACE(
                "VirtAddress:", "[", (void*)(range.base), ",", (void*)(range.end()), ")");
            LOG_TRACE("PhysAddress:",
                      (void*)tos::cur_arch::page_to_address(entry.page_num()));
            char perm_string[4] = "R__";
            auto perms = tos::cur_arch::translate_permissions(entry);
            if (tos::util::is_flag_set(perms, tos::permissions::write)) {
                perm_string[1] = 'W';
            }
            if (tos::util::is_flag_set(perms, tos::permissions::execute)) {
                perm_string[2] = 'X';
            }
            LOG_TRACE("Perms:", perm_string, "User:", entry.allow_user());
        });
}

extern "C" {
void abort() {
    LOG_ERROR("Abort called");
    while (true)
        ;
}
}

void thread() {
    auto uart_res = tos::x86_64::uart_16550::open();
    if (!uart_res) {
        tos::debug::panic("Could not open the uart");
    }
    auto& uart = force_get(uart_res);

    tos::x86_64::text_vga vga;
    vga.clear();
    tos::println(vga, "Hello amd64 Tos!");

    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    LOG("Hello world!");

    uint32_t cpuid_data[4];
    __get_cpuid(0, &cpuid_data[0], &cpuid_data[1], &cpuid_data[2], &cpuid_data[3]);

    char manufacturer_name[12];
    memcpy(&manufacturer_name[0], &cpuid_data[1], 4);
    memcpy(&manufacturer_name[4], &cpuid_data[3], 4);
    memcpy(&manufacturer_name[8], &cpuid_data[2], 4);
    LOG(manufacturer_name);

    auto cr3 = tos::x86_64::read_cr3();
    LOG("Page table at:", (void*)cr3);

    LOG("Image ends at", (void*)tos::default_segments::image().end());

    auto allocator_space = tos::physical_page_allocator::size_for_pages(1024);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto palloc = new ((void*)tos::default_segments::image().end()) tos::physical_page_allocator(1024);
    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, 4096});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    palloc->mark_unavailable({});

    for (int i = 0; i < 200; ++i) {
        auto p = palloc->allocate(1, 1);
        LOG(i, palloc->address_of(*p), palloc->remaining_page_count());
    }

    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 32; ++j) {
            auto vendor_id = tos::x86_64::pci::get_vendor(i, j, 0);
            if (vendor_id != 0xFFFF) {
                auto dev = tos::x86_64::pci::device(i, j, 0);

                LOG("PCI Device at",
                    i,
                    j,
                    (void*)tos::x86_64::pci::get_header_type(i, j, 0),
                    (void*)dev.vendor(),
                    (void*)dev.device_id(),
                    (void*)dev.class_code(),
                    (void*)tos::x86_64::pci::get_subclass(i, j, 0),
                    (void*)tos::x86_64::pci::get_subsys_id(i, j, 0),
                    (void*)dev.status(),
                    "IRQ",
                    int(dev.irq_line()),
                    "BAR0",
                    (void*)dev.bar0(),
                    "BAR1",
                    (void*)dev.bar1(),
                    "BAR4",
                    (void*)dev.bar4(),
                    "BAR5",
                    (void*)dev.bar5(),
                    dev.has_capabilities());

                if (vendor_id == 0x1AF4) {
                    switch (dev.device_id()) {
                    case 0x1001:
                    {
                        LOG("Virtio block device");
                        auto bd = new tos::virtio::block_device(std::move(dev));
                        bd->initialize(palloc);
                        uint8_t buf[512];
                        bd->read(0, buf, 0);
                        bd->read(1, buf, 0);
                        bd->write(0, buf, 0);
                        break;
                    }
                    case 0x1000: {
                        LOG("Virtio network device");
                        auto nd = new tos::virtio::network_device(std::move(dev));
                        nd->initialize(palloc);
                        LOG("MTU", nd->mtu());
                        LOG((void*)(uintptr_t)nd->address().addr[0],
                            (void*)(uintptr_t)nd->address().addr[1],
                            (void*)(uintptr_t)nd->address().addr[2],
                            (void*)(uintptr_t)nd->address().addr[3],
                            (void*)(uintptr_t)nd->address().addr[4],
                            (void*)(uintptr_t)nd->address().addr[5]);
                        break;
                    }
                    default:
                        LOG_WARN("Unknown virtio type:",
                                 (void*)(uintptr_t)dev.device_id());
                        break;
                    }
                }
            }
        }
    }

    while (true) {
        tos::this_thread::yield();
    }

    auto& level0_table = tos::x86_64::get_current_translation_table();

    dump_table(level0_table);
    //
    //    for (uintptr_t i = 0; i < 0x40000000; ++i) {
    //        auto ptr = (volatile char*)i;
    //        LOG((void*)i, *ptr);
    //    }

    tos::cur_arch::breakpoint();

    LOG("Accessing mapped region");
    *((volatile char*)0x400000 - 1) = 42;

    LOG("Accessing unmapped region");
    *((volatile char*)0x400000) = 42;

    LOG("Done");

    while (true) {
        tos::this_thread::yield();
    }
}

tos::stack_storage store;
void tos_main() {
    tos::launch(store, thread);
}