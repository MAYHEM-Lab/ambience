#include <array>
#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/memory.hpp>
#include <tos/soc/bcm2837.hpp>
#include <tos/stack_storage.hpp>

using tos::aarch64::page_id_t;
using tos::aarch64::table_entry;

using table = tos::aarch64::translation_table;

// defined in MAIR register
static constexpr auto PT_MEM = 0; // normal memory
static constexpr auto PT_DEV = 1; // device MMIO

struct pages {
    table pgd{}, pud{}, pud2{};
    std::array<table, 4> pmd{};
};

alignas(4096) pages page;

extern "C" {
alignas(4096) tos::stack_storage _el1_stack;
}

void mmu_init() {
    using namespace tos::aarch64;
    auto io_region = tos::memory_range{tos::bcm2837::IO_BASE,
                                       tos::bcm2837::IO_END - tos::bcm2837::IO_BASE};
    auto arm_core_region =
        tos::memory_range{tos::bcm2837::ARM_CTL_ADDRESS,
                          tos::bcm2837::ARM_CTL_END - tos::bcm2837::ARM_CTL_ADDRESS};

    tos::memory_range ranges[] = {
        io_region,
        arm_core_region,
        tos::default_segments::data(),
        tos::default_segments::text(),
        tos::default_segments::rodata(),
        tos::default_segments::bss(),
    };

    /**
     * Sets up identity mapping at the first 1 gigabyte of memory.
     */
    auto& l2s = page.pud;
    auto& l3s = page.pmd[0];

    page.pgd[0]
        .zero()
        .page_num(tos::aarch64::address_to_page(&l2s))
        .valid(true)
        .page(true)
        .accessed(true)
        .allow_user(true)
        .shareable(tos::aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    page.pgd[1]
        .zero()
        .page_num(tos::aarch64::address_to_page(&page.pud2))
        .valid(true)
        .page(true)
        .accessed(true)
        .allow_user(true)
        .shareable(tos::aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    {
        auto& pud2 = page.pud2;

        auto page = 512; // 512th 2M page, so @1GB
        pud2[0]
            .zero()
            .page_num(tos::aarch64::address_to_page(page << 21))
            .page(false)
            .accessed(true)
            .allow_user(true)
            .noexec(true);

        if (contains(io_region, page << 21) || contains(arm_core_region, page << 21)) {
            pud2[0].shareable(tos::aarch64::shareable_values::outer).mair_index(PT_DEV);
        } else {
            pud2[0].shareable(tos::aarch64::shareable_values::inner).mair_index(PT_MEM);
        }

        if (std::any_of(std::begin(ranges), std::end(ranges), [page](auto& range) {
                return contains(range, page << 21);
            })) {
            pud2[0].valid(true);
        }
    }

    // identity L2 2M blocks

    l2s[0]
        .zero()
        .page_num(address_to_page(&l3s[0]))
        .valid(true)
        .page(true)
        .accessed(true)
        .allow_user(true)
        .shareable(tos::aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    for (page_id_t page = 1; page < 512; page++) {
        l2s[page]
            .zero()
            .page_num(tos::aarch64::address_to_page(page << 21))
            .page(false)
            .accessed(true)
            .allow_user(true)
            .noexec(true);

        if (contains(io_region, page << 21) || contains(arm_core_region, page << 21)) {
            l2s[page]
                .shareable(tos::aarch64::shareable_values::outer)
                .mair_index(PT_DEV)
                .allow_user(false);
        } else {
            l2s[page].shareable(tos::aarch64::shareable_values::inner).mair_index(PT_MEM);
        }

        if (std::any_of(std::begin(ranges), std::end(ranges), [page](auto& range) {
                return contains(range, page << 21);
            })) {
            l2s[page].valid(true);
        }

        if (contains(tos::default_segments::rodata(), page << 21)) {
            l2s[page].readonly(true).noexec(true);
        }

        if (contains(tos::default_segments::bss(), page << 21)) {
            l2s[page].noexec(true);
        }
    }
    // identity L3
    for (int r = 0; r < 512; r++) {
        if (!std::any_of(std::begin(ranges), std::end(ranges), [r](auto& range) {
                return contains(range, tos::aarch64::page_to_address(r));
            })) {
            continue;
        }

        l3s[r]
            .zero()
            .page_num(r)
            .page(true)
            .accessed(true)
            .shareable(tos::aarch64::shareable_values::inner)
            .mair_index(PT_MEM)
            .valid(true);

        /**
         * We follow the W^X permissions scheme.
         *
         * This means that an address is either writeable or executable, but not both at
         * the same time.
         */

        if (!contains(tos::default_segments::text(), tos::aarch64::page_to_address(r))) {
            l3s[r].noexec(true).allow_user(true);
        } else {
            l3s[r].readonly(true).allow_user(true);
        }

        if (contains(tos::default_segments::rodata(), tos::aarch64::page_to_address(r))) {
            l3s[r].readonly(true).noexec(true);
        }

        if (contains(tos::default_segments::bss(), tos::aarch64::page_to_address(r))) {
            l3s[r].noexec(true);
        }
    }

    // check for 4k granule and at least 36 bits physical address bus */
    auto id_reg = tos::aarch64::get_id_aa64mmfr0_el1();
    //    if (id_reg.TGran4 != 0xF || id_reg.PARange == tos::aarch64::pa_ranges::bits_32)
    //    {
    //        return;
    //    }

    // Memory Attributes array, indexed by PT_MEM, PT_DEV
    auto r = (0xFF << 0) | // AttrIdx=0: normal, IWBWA, OWBWA, NTR
             (0x04 << 8);  // AttrIdx=1: device, nGnRE (must be OSH too);
    tos::aarch64::set_mair_el1(r);

    // https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/tcr_el1
    r = (1LLU << 37U) |                    // Top byte ignore
        ((uint64_t)id_reg.PARange << 32) | // IPS=autodetected
        (0b00LL << 14) |                   // TG0=4k
        (0b11LL << 12) |                   // SH0=3 inner
        (0b01LL << 10) |                   // ORGN0=1 write back
        (0b01LL << 8) |                    // IRGN0=1 write back
        (0b0LL << 7) |                     // EPD0, perform translation on TLB miss
        (25LL << 0);                       // T0SZ=25, 3 levels (512G)
    tos::aarch64::set_tcr_el1(r);
    tos::aarch64::isb();

    tos::aarch64::set_ttbr0_el1(&page);

    // finally, toggle some bits in system control register to enable page translation

    asm volatile("dsb ish");
    //    tos::aarch64::dsb();
    tos::aarch64::isb();
    r = tos::aarch64::get_sctlr_el1();
    r |= 0xC00800;     // set mandatory reserved bits
    r &= ~((1 << 25) | // clear EE, little endian translation tables
           (1 << 24) | // clear E0E
           (1 << 12) | // clear I, no instruction cache
           (1 << 2) |  // clear C, no cache at all
           0);
    //    r |= 1 << 1;  // set A
    //    r |= 1 << 4;  // set SA0
    //    r |= 1 << 3;  // set SA
    r |= 1 << 19; // set WXN
    r |= 1 << 0;  // set M, enable MMU

    tos::aarch64::set_sctlr_el1(r);
    tos::aarch64::isb();
}
