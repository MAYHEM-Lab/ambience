#include "kernel.hpp"
#include <calc_generated.hpp>
#include <common/clock.hpp>
#include <common/timer.hpp>
#include <deque>
#include <group1.hpp>
#include <lai/core.h>
#include <lai/helpers/pc-bios.h>
#include <lwip/dhcp.h>
#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <nonstd/variant.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/arch.hpp>
#include <tos/components/allocator.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/elf.hpp>
#include <tos/gnu_build_id.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <tos/mem_stream.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/task.hpp>
#include <tos/virtio/block_device.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/virtio/x86_pci_transport.hpp>
#include <tos/x86_64/apic.hpp>
#include <tos/x86_64/backtrace.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/pit.hpp>
#include <tos/x86_64/syscall.hpp>

extern "C" {
void abort() {
    LOG_ERROR("Abort called");
    while (true)
        ;
}
}

int acpi_ver = 0;
tos::physical_page_allocator* g_palloc;
acpi_rsdt_t* globalRsdtWindow;

namespace {
using page_alloc_res = mpark::variant<tos::cur_arch::mmu_errors>;
using errors = mpark::variant<page_alloc_res, nullptr_t>;

tos::expected<tos::physical_page_allocator*, page_alloc_res> initialize_page_allocator() {
    constexpr auto page_num = 2048;
    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space =
        tos::align_nearest_up_pow2(tos::physical_page_allocator::size_for_pages(page_num),
                                   tos::cur_arch::page_size_bytes);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    EXPECTED_TRYV(
        tos::cur_arch::allocate_region(tos::cur_arch::get_current_translation_table(),
                                       allocator_segment,
                                       tos::user_accessible::no,
                                       nullptr));

    EXPECTED_TRYV(
        tos::cur_arch::mark_resident(tos::cur_arch::get_current_translation_table(),
                                     allocator_segment.range,
                                     tos::memory_types::normal,
                                     vmem_end));

    auto palloc = new (vmem_end) tos::physical_page_allocator(page_num);

    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, tos::cur_arch::page_size_bytes});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    return palloc;
}

void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
    asm volatile("add $8, %rsp");
    asm volatile("movq $0, %rbp");
    asm volatile("movq $0x202, %r11");
    sysret(user_code);
}

struct on_demand_interrupt {
    template<class T>
    void operator()(T&& t) {
        tos::platform::set_irq(12, tos::platform::irq_handler_t(t));
        tos::cur_arch::int0x2c();
    }
};

tos::expected<void, errors> map_elf(const tos::elf::elf64& elf,
                                    tos::physical_page_allocator& palloc,
                                    tos::cur_arch::translation_table& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        LOG("Load",
            (void*)(uint32_t)pheader.file_size,
            "bytes from",
            (void*)(uint32_t)pheader.file_offset,
            "to",
            (void*)(uint32_t)pheader.virt_address);

        auto seg = elf.segment(pheader);

        void* base = const_cast<uint8_t*>(seg.data());
        if (pheader.file_size < pheader.virt_size) {
            auto pages = pheader.virt_size / tos::cur_arch::page_size_bytes;
            auto p = palloc.allocate(pages);
            base = palloc.address_of(*p);
        }

        LOG((void*)pheader.virt_address,
            pheader.virt_size,
            (void*)pheader.file_offset,
            pheader.file_size,
            base);

        auto vseg =
            tos::segment{.range = {.base = pheader.virt_address,
                                   .size = static_cast<ptrdiff_t>(pheader.virt_size)},
                         .perms = tos::permissions::all};

        EXPECTED_TRYV(tos::cur_arch::map_region(root_table,
                                                vseg,
                                                tos::user_accessible::yes,
                                                tos::memory_types::normal,
                                                &palloc,
                                                base));
    }

    return {};
}

tos::expected<tos::span<uint8_t>, errors>
create_and_map_stack(size_t stack_size,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::translation_table& root_table) {
    stack_size = tos::align_nearest_up_pow2(stack_size, tos::cur_arch::page_size_bytes);
    auto page_count = stack_size / tos::cur_arch::page_size_bytes;
    auto stack_pages = palloc.allocate(page_count);

    if (!stack_pages) {
        return tos::unexpected(tos::cur_arch::mmu_errors::page_alloc_fail);
    }

    EXPECTED_TRYV(tos::cur_arch::map_region(
        root_table,
        tos::segment{.range = {.base = reinterpret_cast<uintptr_t>(
                                   palloc.address_of(*stack_pages)),
                               .size = static_cast<ptrdiff_t>(stack_size)},
                     tos::permissions::read_write},
        tos::user_accessible::yes,
        tos::memory_types::normal,
        &palloc,
        palloc.address_of(*stack_pages)));

    return tos::span<uint8_t>(static_cast<uint8_t*>(palloc.address_of(*stack_pages)),
                              stack_size);
}

tos::expected<tos::ae::kernel::user_group, errors> start_group(
    tos::span<uint8_t> stack, void (*entry)(), tos::interrupt_trampoline& trampoline) {
    LOG("Entry point:", (void*)entry);
    auto& user_thread = tos::suspended_launch(stack, switch_to_user, (void*)entry);

    auto& self = *tos::self();

    tos::ae::kernel::user_group res;
    res.state = &user_thread;
    auto syshandler = [&](tos::cur_arch::syscall_frame& frame) {
        assert(frame.rdi == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.rsi);
        res.iface.user_iface = ifc;

        trampoline.switch_to(self);
    };
    tos::x86_64::set_syscall_handler(tos::cur_arch::syscall_handler_t(syshandler));

    tos::swap_context(self, user_thread, tos::int_guard{});
    return res;
}

tos::expected<tos::ae::kernel::user_group, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table) {
    EXPECTED_TRYV(map_elf(elf, palloc, root_table));

    return start_group(EXPECTED_TRY(create_and_map_stack(
                           4 * tos::cur_arch::page_size_bytes, palloc, root_table)),
                       reinterpret_cast<void (*)()>(elf.header().entry),
                       trampoline);
}

class virtio_net_if {
public:
    virtio_net_if(tos::virtio::network_device* dev)
        : m_dev(std::move(dev)) {
        netif_add(
            &m_if, nullptr, nullptr, nullptr, this, &virtio_net_if::init, netif_input);
    }

    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
        if (auto res = dhcp_start(&m_if); res != ERR_OK) {
            LOG_ERROR("No DHCP!", res);
        } else {
            LOG("DHCP Started");
        }
    }

    void down() {
        netif_set_down(&m_if);
        netif_set_link_down(&m_if);
    }

    ~virtio_net_if() {
        down();
        netif_remove(&m_if);
    }

private:
    netif m_if;
    tos::virtio::network_device* m_dev;

    err_t init() {
        LOG("In init");
        m_if.hostname = "ambience";
        m_if.flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        m_if.linkoutput = &virtio_net_if::link_output;
        m_if.output = etharp_output;
        m_if.name[1] = m_if.name[0] = 'm';
        m_if.num = 0;
        m_if.mtu = 1514;
        LOG("MTU", m_if.mtu);
        m_if.hwaddr_len = 6;
        m_if.link_callback = &virtio_net_if::link_callback;
        m_if.status_callback = &virtio_net_if::status_callback;
        auto mac = m_dev->address();
        memcpy(m_if.hwaddr, mac.addr.data(), 6);
        auto& t = launch(tos::alloc_stack,
                         [this] { read_thread(tos::cancellation_token::system()); });
        set_name(t, "LWIP Read Thread");
        return ERR_OK;
    }

    void read_thread(tos::cancellation_token& tok) {
        bool printed = false;
        LOG("In read thread");
        while (!tok.is_cancelled()) {
            auto packet = m_dev->take_packet();

            auto p =
                pbuf_alloc(pbuf_layer::PBUF_RAW, packet.size(), pbuf_type::PBUF_POOL);
            if (!p) {
                LOG_ERROR("Could not allocate pbuf!");
                m_dev->return_packet(packet);
                continue;
            }

            tos::lwip::copy_to_pbuf(packet.begin(), packet.end(), *p);
            m_dev->return_packet(packet);

            {
                tos::lock_guard lg{tos::lwip::lwip_lock};
                m_if.input(p, &m_if);
            }

            if (!printed && dhcp_supplied_address(&m_if)) {
                printed = true;
                auto addr = tos::lwip::convert_to_tos(m_if.ip_addr);
                LOG("Got addr!",
                    (int)addr.addr[0],
                    (int)addr.addr[1],
                    (int)addr.addr[2],
                    (int)addr.addr[3]);
            }
        }
    }

    err_t link_output(pbuf* p) {
        //        LOG_TRACE("link_output", p->len, p->tot_len, "bytes");
        //        m_dev->transmit_packet({static_cast<const uint8_t*>(p->payload),
        //        p->len});
        m_dev->transmit_gather_callback([p]() mutable {
            if (!p) {
                return tos::span<const uint8_t>(nullptr);
            }
            auto res =
                tos::span<const uint8_t>{static_cast<const uint8_t*>(p->payload), p->len};
            p = p->next;
            return res;
        });
        //        LOG_TRACE("link_output", p->len, "bytes done");
        return ERR_OK;
    }

    err_t output(pbuf* p, [[maybe_unused]] const ip4_addr_t* ipaddr) {
        LOG_TRACE("output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        return ERR_OK;
    }

    void link_callback() {
    }

    void status_callback() {
    }

private:
    static err_t init(struct netif* netif) {
        return static_cast<virtio_net_if*>(netif->state)->init();
    }

    static err_t link_output(struct netif* netif, struct pbuf* p) {
        return static_cast<virtio_net_if*>(netif->state)->link_output(p);
    }

    static err_t output(struct netif* netif, struct pbuf* p, const ip4_addr_t* ipaddr) {
        return static_cast<virtio_net_if*>(netif->state)->output(p, ipaddr);
    }

    static void link_callback(struct netif* netif) {
        static_cast<virtio_net_if*>(netif->state)->link_callback();
    }

    static void status_callback(struct netif* netif) {
        static_cast<virtio_net_if*>(netif->state)->status_callback();
    }

    friend void set_default(virtio_net_if& interface) {
        netif_set_default(&interface.m_if);
    }
};

void init_pci(tos::physical_page_allocator& palloc) {
    lwip_init();
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 32; ++j) {
            auto vendor_id = tos::x86_64::pci::get_vendor(i, j, 0);
            if (vendor_id != 0xFFFF) {
                auto dev = tos::x86_64::pci::device(i, j, 0);

                LOG("PCI Device at",
                    i,
                    j,
                    (void*)(uintptr_t)tos::x86_64::pci::get_header_type(i, j, 0),
                    (void*)(uintptr_t)dev.vendor(),
                    (void*)(uintptr_t)dev.device_id(),
                    (void*)dev.class_code(),
                    (void*)(uintptr_t)tos::x86_64::pci::get_subclass(i, j, 0),
                    (void*)(uintptr_t)tos::x86_64::pci::get_subsys_id(i, j, 0),
                    (void*)(uintptr_t)dev.status(),
                    "IRQ",
                    int(dev.irq_line()),
                    "BAR0",
                    (void*)(uintptr_t)dev.bar0(),
                    "BAR1",
                    (void*)(uintptr_t)dev.bar1(),
                    "BAR2",
                    (void*)(uintptr_t)dev.bar2(),
                    "BAR3",
                    (void*)(uintptr_t)dev.bar3(),
                    "BAR4",
                    (void*)(uintptr_t)dev.bar4(),
                    "BAR5",
                    (void*)(uintptr_t)dev.bar5(),
                    dev.has_capabilities());

                auto irq = dev.irq_line();
                if (vendor_id == 0x1AF4) {
                    switch (dev.device_id()) {
                    case 0x1001: {
                        LOG("Virtio block device");
                        break;
                        auto bd = new tos::virtio::block_device(
                            tos::virtio::make_x86_pci_transport(std::move(dev)));
                        bd->initialize(&palloc);
                        uint8_t buf[512];
                        bd->read(0, buf, 0);
                        bd->read(1, buf, 0);
                        bd->write(0, buf, 0);
                        break;
                    }
                    case 0x1000: {
                        LOG("Virtio network device");
                        auto nd = new tos::virtio::network_device(
                            tos::virtio::make_x86_pci_transport(std::move(dev)));
                        nd->initialize(&palloc);
                        LOG("MTU", nd->mtu(), int(irq));
                        LOG((void*)(uintptr_t)nd->address().addr[0],
                            (void*)(uintptr_t)nd->address().addr[1],
                            (void*)(uintptr_t)nd->address().addr[2],
                            (void*)(uintptr_t)nd->address().addr[3],
                            (void*)(uintptr_t)nd->address().addr[4],
                            (void*)(uintptr_t)nd->address().addr[5]);
                        auto interface = new virtio_net_if(nd);
                        set_default(*interface);
                        interface->up();

                        return;
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
}
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

struct scrollable : tos::self_pointing<scrollable> {
    void draw() {
        auto lines_to_draw = std::min<int>(m_vga.heigth, m_lines.size() - m_top_line);
        for (int i = m_top_line, j = 0; j < lines_to_draw; ++i, ++j) {
            m_vga.write_line(j, std::string_view(m_lines[i]));
        }
    }

    int write(tos::span<const uint8_t> data) {
        for (auto c : data) {
            if (c == '\n') {
                m_lines.emplace_back();
                m_lines.back().reserve(48);

                if (m_locked && m_lines.size() > m_vga.heigth) {
                    m_top_line += 1;
                }
                continue;
            } else if (c == '\r' || c == 0) {
                continue;
            }
            m_lines.back() += c;
        }
        draw();
        return data.size();
    }

    bool scroll_down() {
        if (m_top_line == m_lines.size() - 1) {
            return false;
        }
        m_top_line += 1;
        if (m_top_line == m_lines.size() - 1) {
            m_locked = true;
        }
        draw();
        return true;
    }

    bool scroll_up() {
        if (m_top_line == 0) {
            return false;
        }
        m_top_line -= 1;
        m_locked = false;
        draw();
        return true;
    }

    bool m_locked = true;
    int m_top_line = 0;
    std::vector<std::string> m_lines = std::vector<std::string>(1);
    tos::x86_64::text_vga m_vga{};
};

tos::x86_64::text_vga vga{};

void kb_isr(tos::x86_64::exception_frame* frame, int) {
    auto kc = tos::x86_64::port(0x60).inb();
    if (kc == 36) {        // j
                           //        vga.scroll_up();
    } else if (kc == 37) { // k
                           //        vga.scroll_down();
    }
}

class x86_64_platform_support {
public:
    void stage1_init() {
    }

    void stage2_init() {
        auto apic_base = tos::x86_64::get_apic_base_address();
        LOG((void*)apic_base,
            (void*)tos::x86_64::rdmsr(tos::x86_64::msrs::ia32_apic_base));
        auto& table = tos::cur_arch::get_current_translation_table();
        auto seg = tos::segment{
            .range = {.base = apic_base, .size = tos::cur_arch::page_size_bytes},
            .perms = tos::permissions::read_write};
        Assert(tos::x86_64::map_region(table,
                                       seg,
                                       tos::user_accessible::no,
                                       tos::memory_types::device,
                                       m_palloc,
                                       reinterpret_cast<void*>(apic_base)));

        seg = tos::segment{
            .range = {.base = 0xfec00000, .size = tos::cur_arch::page_size_bytes},
            .perms = tos::permissions::read_write};
        Assert(tos::x86_64::map_region(table,
                                       seg,
                                       tos::user_accessible::no,
                                       tos::memory_types::device,
                                       m_palloc,
                                       reinterpret_cast<void*>(0xfec00000)));

        auto& apic_regs = tos::x86_64::get_apic_registers(apic_base);
        LOG((void*)(uintptr_t)apic_regs.id, (void*)(uintptr_t)apic_regs.version);

        g_palloc = m_palloc;
        lai_rsdp_info rsdp_info;
        auto res = lai_bios_detect_rsdp(&rsdp_info);
        LOG(res,
            rsdp_info.acpi_version,
            (void*)rsdp_info.rsdp_address,
            (void*)rsdp_info.rsdt_address);
        acpi_ver = rsdp_info.acpi_version;

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

        tos::x86_64::pic::disable();
        tos::x86_64::apic apic(apic_regs);
        apic.enable();
        LOG(apic_regs.tpr);
        apic_regs.tpr = 0;

        auto IOAPICID = tos::x86_64::ioapic_read((void*)0xfec00000, 0);
        LOG(IOAPICID);
        auto IOAPICVER = tos::x86_64::ioapic_read((void*)0xfec00000, 1);
        LOG((void*)IOAPICVER);

        tos::x86_64::ioapic_set_irq(2, apic_regs.id, 32 + 0);
        tos::x86_64::ioapic_set_irq(1, apic_regs.id, 32 + 1);

        tos::platform::set_irq(1, tos::free_function_ref(+kb_isr));

        tos::lwip::global::system_clock = &m_clock;
        init_pci(*m_palloc);
    }

    auto init_serial() {
        //        return &vga;
        auto ser = tos::x86_64::uart_16550::open();
        if (!ser) {
            tos::println(vga, "Could not open uart");
            while (true)
                ;
        }
        return force_get(ser);
    }

    struct timer
        : tos::x86_64::pit
        , tos::self_pointing<timer> {
        timer() {
            tos::platform::set_irq(0, tos::mem_function_ref<&timer::irq>(*this));
        }

        timer(const timer&) = delete;
        timer(timer&&) = delete;

        void enable() {
            //            tos::x86_64::pic::enable_irq(0);
        }

        void disable() {
            //            tos::x86_64::pic::disable_irq(0);
        }

        void set_callback(tos::function_ref<void()> cb) {
            m_cb = cb;
        }

        void irq(tos::x86_64::exception_frame* frame, int) {
            m_cb();
        }

        tos::function_ref<void()> m_cb{[](void*) {}};
    };

    auto init_timer() {
        return m_tim_mux.channel(0);
    }

    auto init_clock() {
    }

    auto init_odi() {
        return on_demand_interrupt();
    }

    auto init_physical_memory_allocator() {
        auto palloc = force_get(initialize_page_allocator());
        m_palloc = palloc;
        return palloc;
    }

    std::vector<tos::ae::kernel::user_group>
    init_groups(tos::interrupt_trampoline& trampoline,
                tos::physical_page_allocator& palloc) {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

        std::vector<tos::ae::kernel::user_group> runnable_groups;
        {
            LOG(group1.slice(0, 4));
            auto elf_res = tos::elf::elf64::from_buffer(group1);
            if (!elf_res) {
                LOG_ERROR("Could not parse payload!");
                LOG_ERROR("Error code: ", int(force_error(elf_res)));
                while (true)
                    ;
            }

            runnable_groups.push_back(force_get(
                load_from_elf(force_get(elf_res), trampoline, palloc, level0_table)));
            LOG("Group loaded");

            runnable_groups.back().channels.push_back(
                std::make_unique<tos::ae::services::calculator::async_zerocopy_client<
                    tos::ae::downcall_transport>>(
                    *runnable_groups.back().iface.user_iface, 0));
        }

        return runnable_groups;
    }

    template<class FnT>
    void set_syscall_handler(FnT&& syshandler) {
        tos::x86_64::set_syscall_handler(tos::x86_64::syscall_handler_t(syshandler));
    }

    void return_to_thread_from_irq(tos::kern::tcb& from, tos::kern::tcb& to) {
        return_from = &from;
        return_to = &to;
        tos::platform::set_post_irq(
            tos::mem_function_ref<&x86_64_platform_support::do_return>(*this));
    }

    void do_return(tos::x86_64::exception_frame*) {
        tos::platform::reset_post_irq();
        tos::swap_context(*return_from, *return_to, tos::int_ctx{});
    }

    tos::kern::tcb* return_from;
    tos::kern::tcb* return_to;

    tos::physical_page_allocator* m_palloc;

    using timer_mux_type = tos::timer_multiplexer<timer, 3>;
    using channel_type = timer_mux_type::multiplexed_timer;
    using clock_type = tos::clock<channel_type>;
    using erased_clock_type = tos::detail::erased_clock<clock_type>;

    using alarm_type = tos::alarm<channel_type>;
    using erased_alarm_type = tos::detail::erased_alarm<alarm_type>;

    timer_mux_type m_tim_mux;
    erased_clock_type m_clock{m_tim_mux.channel(1)};
    erased_alarm_type m_alarm{m_tim_mux.channel(2)};
};
} // namespace

tos::expected<void, errors> kernel() {
    tos::ae::manager<x86_64_platform_support> man;
    man.initialize();

    set_name(tos::launch(tos::alloc_stack,
                         [&] {
                             while (true) {
                                 using namespace std::chrono_literals;
                                 tos::this_thread::sleep_for(man.m_alarm, 500ms);
                                 tos::lock_guard lg{tos::lwip::lwip_lock};
                                 sys_check_timeouts();
                             }
                         }),
             "LWIP check timeouts");

    tos::debug::log_server serv(man.get_log_sink());

    man.groups().front().exposed_services.emplace_back(&serv);

    auto req_task = [&g = man.groups().front()]() -> tos::Task<void> {
        auto serv = static_cast<tos::ae::services::calculator::async_server*>(
            g.channels.front().get());

        LOG("3 + 4 =", co_await serv->add(3, 4));
    };

    tos::coro_job j(tos::current_context(), tos::coro::make_pollable(req_task()));
    tos::kern::make_runnable(j);
    tos::this_thread::yield();

    for (int i = 0; i < 10; ++i) {
        man.run();
    }

    tos::lwip::tcp_socket sock(tos::port_num_t{80});

    tos::mutex backlog_mut;
    tos::semaphore sem{0};
    std::deque<std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>>> backlog;
    auto handler = [&](auto& sock, tos::lwip::tcp_endpoint&& ep) {
        tos::lock_guard lg(backlog_mut);
        if (backlog.size() > 20) {
            LOG("Busy!");
            return false;
        }
        backlog.emplace_back(
            std::make_unique<tos::tcp_stream<tos::lwip::tcp_endpoint>>(std::move(ep)));
        sem.up();
        return true;
    };

    int x = 0;
    auto handle_req =
        [&](int id, std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>> sockstr) {
            auto begin = tos::lwip::global::system_clock->now();
            std::array<uint8_t, 1024> buf;
            tos::omemory_stream str(buf);
            tos::println(str, "HTTP/1.0 200 Content-type: text/html");
            tos::println(str);
            tos::println(str, "<body>");
            tos::println(str, "<h1>Hello from ambience!</h1>");
            tos::println(str, "Tos build name:", build_id.name(), "<br />");
            tos::println(str, "Tos build id:", build_id.id(), "<br />");
            tos::println(str, "Request num:", x++, "<br />");
            tos::println(str, "Worker id:", id, "<br />");
            tos::println(str, "Backlog size:", backlog.size(), "<br />");
            auto* alloc =
                tos::current_context().get_component<tos::allocator_component>();
            tos::println(str,
                         "Heap memory in use:",
                         alloc->allocator->in_use().value_or(-1),
                         "<br />");
            tos::println(str,
                         "Uptime:",
                         (int)std::chrono::duration_cast<std::chrono::seconds>(
                             tos::lwip::global::system_clock->now().time_since_epoch())
                             .count(),
                         "seconds",
                         "<br />");
            tos::println(str,
                         "Req took:",
                         (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                             tos::lwip::global::system_clock->now() - begin)
                             .count(),
                         "milliseconds",
                         "<br />");
            tos::println(str, "</body>");
            sockstr->write(str.get());
        };

    auto thread = [&](int worker_id, tos::cancellation_token& tok) {
        while (!tok.is_cancelled()) {
            sem.down(tok);
            backlog_mut.lock();
            auto sock = std::move(backlog.front());
            backlog.pop_front();
            backlog_mut.unlock();
            handle_req(worker_id, std::move(sock));
        }
    };

    for (int i = 0; i < 5; ++i) {
        auto& t =
            tos::launch(tos::alloc_stack, thread, i, tos::cancellation_token::system());
        set_name(t, "HTTP Thread Pool Worker");
    }

    // thread(4, tos::cancellation_token::system());

    sock.async_accept(handler);

    tos::this_thread::block_forever();

    return {};
}

// struct log_handler : tos::ubsan::handlers::ubsan_handlers {
//     void error() override {
//         LOG_ERROR("ERROR!");
//         LOG_ERROR("Call stack:");
//         tos::kern::processor_state state;
//         save_ctx(state);
//
//         auto root = std::optional<tos::x86_64::trace_elem>{
//             {.rbp = tos::x86_64::read_rbp(), .rip = tos::x86_64::get_rip(state.buf)}};
//         while (root) {
//             LOG_ERROR((void*)root->rip);
//             root = backtrace_next(*root);
//         }
//
//         while (true)
//             ;
//         //
//         //        namespace tos::ubsan::handlers {
//         //            void nonnull_arg() {
//         //                LOG_ERROR("nonnull arg!");
//         //                while (true);
//         //            }
//         //
//         //            void load_invalid_value() {
//         //                LOG_ERROR("invalid value!");
//         //                while (true);
//         //            }
//         //
//         //            void type_mismatch_v1() {
//         //                LOG_ERROR("type_mismatch_v1!");
//         //                while (true);
//         //            }
//         //        }
//     }
// };

// log_handler handler_;
static tos::stack_storage kern_stack;
void tos_main() {
    //    tos::ubsan::handlers::handlers = &handler_;
    set_name(tos::launch(kern_stack, kernel), "Kernel");
}