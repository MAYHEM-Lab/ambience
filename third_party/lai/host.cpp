#include <acpispec/tables.h>
#include <lai/host.h>
#include <tos/debug/log.hpp>
#include <tos/debug/panic.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/port.hpp>

extern tos::physical_page_allocator* g_palloc;
extern acpi_rsdt_t* globalRsdtWindow;
extern int acpi_ver;

extern "C" {
void laihost_log(int level, const char* msg) {
    tos::debug::log(level, msg);
}

void laihost_panic(const char* msg) {
    tos::debug::panic(msg);
}

void* laihost_malloc(size_t sz) {
    return malloc(sz);
}

void* laihost_realloc(void* ptr, size_t newsize, size_t oldsize) {
    auto new_buf = malloc(newsize);
    if (!new_buf) {
        free(ptr);
        return nullptr;
    }
    memcpy(new_buf, ptr, oldsize);
    free(ptr);
    return new_buf;
}

void laihost_free(void* ptr, size_t) {
    free(ptr);
}

void* laihost_map(size_t address, size_t count) {
    LOG_TRACE("map", (void*)address, (void*)count);
    auto addr_bkp = address;
    address = tos::align_nearest_down_pow2(address, tos::x86_64::page_size_bytes);
    count = tos::align_nearest_up_pow2(count, tos::x86_64::page_size_bytes);
    LOG_TRACE("map", (void*)address, (void*)count);

    auto& root = tos::x86_64::get_current_translation_table();
    auto segment = tos::segment{.range = {.base = address, .size = ptrdiff_t(count)},
                                .perms = tos::permissions::read_write};
    auto res =
        tos::x86_64::allocate_region(root, segment, tos::user_accessible::no, g_palloc);

    if (res || force_error(res) == tos::x86_64::mmu_errors::already_allocated) {
        res = tos::x86_64::mark_resident(
            root, segment.range, tos::memory_types::normal, tos::physical_address{address});
        if (res) {
            LOG_TRACE("returning", (void*)addr_bkp);

            return (void*)addr_bkp;
        }
    }

    LOG_ERROR("returning nullptr", int(force_error(res)));

    return nullptr;
}

void laihost_unmap(void* pointer, size_t count) {
    pointer = tos::align_nearest_down_pow2(pointer, tos::x86_64::page_size_bytes);
    count = tos::align_nearest_up_pow2(count, tos::x86_64::page_size_bytes);
    auto range = tos::memory_range{.base = reinterpret_cast<uintptr_t>(pointer),
                                   .size = ptrdiff_t(count)};
    auto& root = tos::x86_64::get_current_translation_table();

    ensure(tos::x86_64::mark_nonresident(root, range));
}

static void* mapTable(uintptr_t address) {
    auto headerWindow = laihost_map(address, sizeof(acpi_header_t));
    auto headerPtr = reinterpret_cast<acpi_header_t*>(headerWindow);
    auto len = headerPtr->length;
    laihost_unmap(headerPtr, sizeof(acpi_header_t));
    return laihost_map(address, len);
}

static void* scanRsdt(const char* name, size_t index) {
    if (acpi_ver == 1) {
        auto rsdt = globalRsdtWindow;
        Assert(rsdt->header.length >= sizeof(acpi_header_t));

        size_t n = 0;
        int numPtrs = (rsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint32_t);
        for (int i = 0; i < numPtrs; i++) {
            auto tableWindow =
                reinterpret_cast<acpi_header_t*>(mapTable(rsdt->tables[i]));
            Assert(tableWindow);
            char sig[5];
            sig[4] = 0;
            memcpy(sig, tableWindow->signature, 4);
            if (memcmp(tableWindow->signature, name, 4))
                continue;
            if (n == index)
                return tableWindow;
            n++;
        }
    } else if (acpi_ver == 2) {
        auto xsdt = reinterpret_cast<acpi_xsdt_t*>(globalRsdtWindow);
        Assert(xsdt->header.length >= sizeof(acpi_header_t));

        size_t n = 0;
        int numPtrs = (xsdt->header.length - sizeof(acpi_header_t)) / sizeof(uint64_t);
        for (int i = 0; i < numPtrs; i++) {
            auto tableWindow =
                reinterpret_cast<acpi_header_t*>(mapTable(xsdt->tables[i]));
            Assert(tableWindow);
            char sig[5];
            sig[4] = 0;
            memcpy(sig, tableWindow->signature, 4);
            if (memcmp(tableWindow->signature, name, 4))
                continue;
            if (n == index)
                return tableWindow;
            n++;
        }
    }

    return nullptr;
}

void* laihost_scan(const char* name, size_t index) {
    if (!memcmp(name, "DSDT", 4)) {
        void* fadtWindow = scanRsdt("FACP", 0);
        Assert(fadtWindow);
        auto fadt = reinterpret_cast<acpi_fadt_t*>(fadtWindow);
        void* dsdtWindow = mapTable(fadt->dsdt);
        return dsdtWindow;
    } else {
        return scanRsdt(name, index);
    }
}
void laihost_outb(uint16_t addr, uint8_t val) {
    LOG("outb", (void*)(uintptr_t)addr, (void*)(uintptr_t)val);
    return tos::x86_64::port(addr).outb(val);
}
void laihost_outw(uint16_t addr, uint16_t val) {
    LOG("outw", (void*)(uintptr_t)addr, (void*)(uintptr_t)val);
    return tos::x86_64::port(addr).outw(val);
}
void laihost_outd(uint16_t addr, uint32_t val) {
    LOG("outl", (void*)(uintptr_t)addr, (void*)(uintptr_t)val);
    return tos::x86_64::port(addr).outl(val);
}
uint8_t laihost_inb(uint16_t addr) {
    LOG("inb", (void*)(uintptr_t)addr);
    return tos::x86_64::port(addr).inb();
}
uint16_t laihost_inw(uint16_t addr) {
    LOG("inw", (void*)(uintptr_t)addr);
    return tos::x86_64::port(addr).inw();
}
uint32_t laihost_ind(uint16_t addr) {
    LOG("inl", (void*)(uintptr_t)addr);
    return tos::x86_64::port(addr).inl();
}
void laihost_pci_writeb(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t) {
    Assert(false && "laihost_pci_writeb");
}
uint8_t laihost_pci_readb(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t) {
    Assert(false && "laihost_pci_readb");
}
void laihost_pci_writew(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint16_t) {
    Assert(false && "laihost_pci_writew");
}
uint16_t laihost_pci_readw(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t) {
    Assert(false && "laihost_pci_readw");
}
void laihost_pci_writed(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint32_t) {
    Assert(false && "laihost_pci_writed");
}
uint32_t laihost_pci_readd(uint16_t, uint8_t, uint8_t, uint8_t, uint16_t) {
    Assert(false && "laihost_pci_readd");
}
void laihost_sleep(uint64_t) {
    Assert(false && "laihost_sleep");
}
int laihost_sync_wait(struct lai_sync_state*, unsigned int val, int64_t deadline) {
    Assert(false && "laihost_sync_wait");
}
void laihost_sync_wake(struct lai_sync_state*) {
    Assert(false && "laihost_sync_wake");
}
void laihost_handle_amldebug(lai_variable_t*) {
    Assert(false && "laihost_handle_amldebug");
}
}