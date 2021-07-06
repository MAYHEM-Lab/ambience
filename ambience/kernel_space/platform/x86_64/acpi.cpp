#include <cassert>
#include <lai/core.h>
#include <lai/helpers/pc-bios.h>
#include <lai/helpers/pm.h>
#include <tos/debug/log.hpp>

struct [[gnu::packed]] MadtHeader {
    uint32_t localApicAddress;
    uint32_t flags;
};

struct [[gnu::packed]] MadtGenericEntry {
    uint8_t type;
    uint8_t length;
};

struct [[gnu::packed]] MadtLocalEntry {
    MadtGenericEntry generic;
    uint8_t processorId;
    uint8_t localApicId;
    uint32_t flags;
};

namespace local_flags {
static constexpr uint32_t enabled = 1;
};

struct [[gnu::packed]] MadtIoEntry {
    MadtGenericEntry generic;
    uint8_t ioApicId;
    uint8_t reserved;
    uint32_t mmioAddress;
    uint32_t systemIntBase;
};

enum OverrideFlags
{
    polarityMask = 0x03,
    polarityDefault = 0x00,
    polarityHigh = 0x01,
    polarityLow = 0x03,

    triggerMask = 0x0C,
    triggerDefault = 0x00,
    triggerEdge = 0x04,
    triggerLevel = 0x0C
};

struct [[gnu::packed]] MadtIntOverrideEntry {
    MadtGenericEntry generic;
    uint8_t bus;
    uint8_t sourceIrq;
    uint32_t systemInt;
    uint16_t flags;
};

struct [[gnu::packed]] MadtLocalNmiEntry {
    MadtGenericEntry generic;
    uint8_t processorId;
    uint16_t flags;
    uint8_t localInt;
};

void dumpMadt() {
    void* madtWindow = laihost_scan("APIC", 0);
    assert(madtWindow);
    auto madt = reinterpret_cast<acpi_header_t*>(madtWindow);

    size_t offset = sizeof(acpi_header_t) + sizeof(MadtHeader);
    while (offset < madt->length) {
        auto generic = (MadtGenericEntry*)((uintptr_t)madt + offset);
        if (generic->type == 0) { // local APIC
            auto entry = (MadtLocalEntry*)generic;
            LOG("Local APIC id:",
                (int)entry->localApicId,
                ((entry->flags & local_flags::enabled) ? "" : " (disabled)"));
        } else if (generic->type == 1) { // I/O APIC
            auto entry = (MadtIoEntry*)generic;
            LOG("I/O APIC id:",
                (int)entry->ioApicId,
                ", sytem interrupt base:",
                (int)entry->systemIntBase,
                (void*)(uintptr_t)entry->mmioAddress);
        } else if (generic->type == 2) { // interrupt source override
            auto entry = (MadtIntOverrideEntry*)generic;

            const char *bus, *polarity, *trigger;
            if (entry->bus == 0) {
                bus = "ISA";
            } else {
                LOG_ERROR("Unexpected bus in MADT interrupt override");
            }

            if ((entry->flags & OverrideFlags::polarityMask) ==
                OverrideFlags::polarityDefault) {
                polarity = "default";
            } else if ((entry->flags & OverrideFlags::polarityMask) ==
                       OverrideFlags::polarityHigh) {
                polarity = "high";
            } else if ((entry->flags & OverrideFlags::polarityMask) ==
                       OverrideFlags::polarityLow) {
                polarity = "low";
            } else {
                LOG_ERROR("Unexpected polarity in MADT interrupt override");
            }

            if ((entry->flags & OverrideFlags::triggerMask) ==
                OverrideFlags::triggerDefault) {
                trigger = "default";
            } else if ((entry->flags & OverrideFlags::triggerMask) ==
                       OverrideFlags::triggerEdge) {
                trigger = "edge";
            } else if ((entry->flags & OverrideFlags::triggerMask) ==
                       OverrideFlags::triggerLevel) {
                trigger = "level";
            } else {
                LOG_ERROR("Unexpected trigger mode in MADT interrupt override");
            }

            LOG("Int override: ",
                bus,
                "IRQ",
                (int)entry->sourceIrq,
                "is mapped to GSI",
                entry->systemInt,
                "(Polarity:",
                polarity,
                ", trigger mode:",
                trigger,
                ")");
        } else if (generic->type == 4) { // local APIC NMI source
            auto entry = (MadtLocalNmiEntry*)generic;
            LOG("Local APIC NMI: processor",
                (int)entry->processorId,
                ", lint:",
                (int)entry->localInt);
        } else {
            LOG("Unexpected MADT entry of type", generic->type);
        }
        offset += generic->length;
    }
}

int acpi_ver = 0;
acpi_rsdt_t* globalRsdtWindow;

void do_acpi_stuff() {
    lai_rsdp_info rsdp_info;
    auto res = lai_bios_detect_rsdp(&rsdp_info);
    LOG(res,
        rsdp_info.acpi_version,
        (void*)rsdp_info.rsdp_address,
        (void*)rsdp_info.rsdt_address);
    acpi_ver = rsdp_info.acpi_version;

    lai_set_acpi_revision(rsdp_info.acpi_version);

    if (rsdp_info.acpi_version == 2) {
        auto win = laihost_map(rsdp_info.xsdt_address, 0x1000);
        globalRsdtWindow = reinterpret_cast<acpi_rsdt_t*>(win);
        //            globalRsdtWindow = laihost_map(rsdp_info.xsdt_address,
        //            xsdt->header.length);
    } else if (rsdp_info.acpi_version == 1) {
        auto win = laihost_map(rsdp_info.rsdt_address, 0x1000);
        globalRsdtWindow = reinterpret_cast<acpi_rsdt_t*>(win);
        //            globalRsdtWindow = laihost_map(rsdp_info.rsdt_address, 0x1000);
        //            auto rsdt = reinterpret_cast<acpi_rsdt_t *>(globalRsdtWindow);
        //            globalRsdtWindow = laihost_map(rsdp_info.rsdt_address,
        //            rsdt->header.length);
    }

    LOG(res,
        rsdp_info.acpi_version,
        std::string_view(globalRsdtWindow->header.signature, 4),
        globalRsdtWindow->header.length,
        globalRsdtWindow->header.revision);

    lai_create_namespace();

    dumpMadt();
    void* madtWindow = laihost_scan("APIC", 0);
    assert(madtWindow);
    auto madt = reinterpret_cast<acpi_header_t*>(madtWindow);
}
