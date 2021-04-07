#include "common/inet/tcp_ip.hpp"
#include "lwip/init.h"
#include "tos/function_ref.hpp"
#include "tos/lwip/udp.hpp"
#include "tos/memory.hpp"
#include "tos/paging/physical_page_allocator.hpp"
#include "tos/platform.hpp"
#include "tos/print.hpp"
#include "tos/semaphore.hpp"
#include "tos/span.hpp"
#include "tos/thread.hpp"
#include "tos/utility.hpp"
#include "tos/x86_64/exception.hpp"
#include <calc_generated.hpp>
#include <cstddef>
#include <deque>
#include <group1.hpp>
#include <group2.hpp>
#include <tos/address_space.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/rings.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/elf.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/lwip/if_adapter.hpp>
#include <tos/mem_stream.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/physical_memory_backing.hpp>
#include <tos/suspended_launch.hpp>
#include <tos/virtio/block_device.hpp>
#include <tos/virtio/network_device.hpp>
#include <tos/virtio/x86_pci_transport.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/cpuid.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/syscall.hpp>

void dump_table(tos::cur_arch::translation_table& table) {
    tos::cur_arch::traverse_table_entries(
        table, [](tos::memory_range range, tos::cur_arch::table_entry& entry) {
            LOG_TRACE((void*)entry.raw());
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

class virtio_net_if {
public:
    virtio_net_if(tos::virtio::network_device* dev,
                  const tos::ipv4_addr_t& addr,
                  const tos::ipv4_addr_t& mask,
                  const tos::ipv4_addr_t& gw)
        : m_dev(std::move(dev)) {
        auto lwip_addr = tos::lwip::convert_address(addr);
        auto lwip_mask = tos::lwip::convert_address(mask);
        auto lwip_gw = tos::lwip::convert_address(gw);
        netif_add(&m_if,
                  &lwip_addr,
                  &lwip_mask,
                  &lwip_gw,
                  this,
                  &virtio_net_if::init,
                  netif_input);
    }

    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
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
        m_if.hostname = "tos";
        m_if.flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        m_if.linkoutput = &virtio_net_if::link_output;
        m_if.output = etharp_output;
        m_if.name[1] = m_if.name[0] = 'm';
        m_if.num = 0;
        m_if.mtu = m_dev->mtu();
        m_if.hwaddr_len = 6;
        m_if.link_callback = &virtio_net_if::link_callback;
        m_if.status_callback = &virtio_net_if::status_callback;
        // std::iota(std::begin(m_if.hwaddr), std::end(m_if.hwaddr), 1);
        auto mac = m_dev->address();
        memcpy(m_if.hwaddr, mac.addr.data(), 6);
        launch(tos::alloc_stack, [this] { read_thread(); });
        return ERR_OK;
    }

    void read_thread() {
        LOG("In read thread");
        auto& tok = tos::cancellation_token::system();
        while (!tok.is_cancelled()) {
            auto packet = m_dev->take_packet();
            // LOG_TRACE("Received", packet.size(), "bytes");
            // LOG_TRACE(packet);
            auto p =
                pbuf_alloc(pbuf_layer::PBUF_RAW, packet.size(), pbuf_type::PBUF_POOL);
            std::copy(packet.begin(), packet.end(), static_cast<uint8_t*>(p->payload));
            m_dev->return_packet(packet);
            m_if.input(p, &m_if);
        }
    }

    err_t link_output(pbuf* p) {
        // LOG_TRACE("link_output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        m_dev->transmit_packet({static_cast<const uint8_t*>(p->payload), p->len});
        // LOG_TRACE("Written bytes");
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

void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
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

void init_pci(tos::physical_page_allocator& palloc) {
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
                    "BAR4",
                    (void*)(uintptr_t)dev.bar4(),
                    "BAR5",
                    (void*)(uintptr_t)dev.bar5(),
                    dev.has_capabilities());

                if (vendor_id == 0x1AF4) {
                    switch (dev.device_id()) {
                    case 0x1001: {
                        LOG("Virtio block device");
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
                        LOG("MTU", nd->mtu());
                        LOG((void*)(uintptr_t)nd->address().addr[0],
                            (void*)(uintptr_t)nd->address().addr[1],
                            (void*)(uintptr_t)nd->address().addr[2],
                            (void*)(uintptr_t)nd->address().addr[3],
                            (void*)(uintptr_t)nd->address().addr[4],
                            (void*)(uintptr_t)nd->address().addr[5]);

                        auto interface =
                            new virtio_net_if(nd,
                                              tos::parse_ipv4_address("10.0.2.15"),
                                              tos::parse_ipv4_address("255.255.255.0"),
                                              tos::parse_ipv4_address("10.0.2.2"));
                        set_default(*interface);
                        interface->up();
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

using error_type = mpark::variant<tos::cur_arch::mmu_errors>;

tos::expected<void, error_type> map_elf(const tos::elf::elf64& elf,
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

tos::expected<tos::span<uint8_t>, error_type>
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

tos::expected<tos::ae::kernel::user_group, error_type>
start_group(tos::span<uint8_t> stack,
            void (*entry)(),
            tos::interrupt_trampoline& trampoline,
            tos::cur_arch::translation_table& root_table) {
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

tos::expected<tos::ae::kernel::user_group, error_type>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table) {
    EXPECTED_TRYV(map_elf(elf, palloc, root_table));

    return start_group(EXPECTED_TRY(create_and_map_stack(
                           4 * tos::cur_arch::page_size_bytes, palloc, root_table)),
                       reinterpret_cast<void (*)()>(elf.header().entry),
                       trampoline,
                       root_table);
}

void thread() {
    auto& self = *tos::self();

    auto uart_res = tos::x86_64::uart_16550::open();
    if (!uart_res) {
        tos::debug::panic("Could not open the uart");
    }
    auto& uart = force_get(uart_res);
    tos::println(uart, "Booted");

    tos::x86_64::text_vga vga;
    vga.clear();
    tos::println(vga, "Hello amd64 Tos!");

    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    tos::x86_64::pic::enable_irq(0);

    LOG("Hello world!");

    LOG(tos::x86_64::cpuid::manufacturer().data());
    on_demand_interrupt odi{};
    auto trampoline = tos::make_interrupt_trampoline(odi);

    auto cr3 = tos::x86_64::read_cr3();
    LOG("Page table at:", (void*)cr3);

    tos::physical_memory_backing pmem(
        tos::segment{tos::memory_range{.base = 0, .size = 1'000'000'000},
                     tos::permissions::all},
        tos::memory_types::normal);

    auto& level0_table = tos::cur_arch::get_current_translation_table();

    dump_table(level0_table);

    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space =
        tos::align_nearest_up_pow2(tos::physical_page_allocator::size_for_pages(1024),
                                   tos::cur_arch::page_size_bytes);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    auto op_res =
        tos::cur_arch::map_region(tos::cur_arch::get_current_translation_table(),
                                  allocator_segment,
                                  tos::user_accessible::no,
                                  tos::memory_types::normal,
                                  nullptr,
                                  vmem_end);
    LOG(bool(op_res));

    auto palloc = new (vmem_end) tos::physical_page_allocator(1024);
    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, tos::cur_arch::page_size_bytes});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    tos::cur_arch::address_space as;
    as.m_table = &level0_table;
    tos::address_space vas(as);
    tos::global::cur_as = &vas;

    auto mapping = pmem.create_mapping(
        tos::segment{
            tos::memory_range{.base = 0x1200000, .size = tos::cur_arch::page_size_bytes},
            tos::permissions::read_write},
        tos::memory_range{.base = 0x200000, .size = tos::cur_arch::page_size_bytes});

    vas.do_mapping(*mapping, palloc);

    LOG(*((int*)0x200000));
    LOG(*((int*)0x1200000));

    lwip_init();

    init_pci(*palloc);

    tos::lwip::async_udp_socket sock;

    auto handler = [&](auto,
                       tos::lwip::async_udp_socket*,
                       tos::udp_endpoint_t from,
                       tos::lwip::buffer buf) {
        std::vector<uint8_t> data(buf.size());
        buf.read(data);
        std::string_view sv(reinterpret_cast<char*>(data.data()), data.size());
        LOG(sv);
        sock.send_to(data, from);
    };

    sock.attach(handler);

    auto bind_res = sock.bind({100}, tos::parse_ipv4_address("10.0.2.15"));
    LOG(bool(bind_res));

    tos::semaphore sem{0};

    auto tim_handler = [&](tos::x86_64::exception_frame* frame, int) {
        LOG("Tick", frame->cs, frame->ss);
        sem.up_isr();
    };

    tos::platform::set_irq(
        0, tos::function_ref<void(tos::x86_64::exception_frame*, int)>(tim_handler));

    std::vector<tos::ae::kernel::user_group> runnable_groups;
    {
        LOG(group1.slice(0, 4));
        auto elf_res = tos::elf::elf64::from_buffer(group1);
        if (!elf_res) {
            vga.write("Could not parse payload\n\r");
            vga.write("Error code: ");
            vga.write(tos::itoa(int(force_error(elf_res))).data());
            vga.write("\n\r");
            while (true)
                ;
        }

        runnable_groups.push_back(force_get(
            load_from_elf(force_get(elf_res), *trampoline, *palloc, level0_table)));
    }

    {
        LOG(group2.slice(0, 4));
        auto elf_res = tos::elf::elf64::from_buffer(group2);
        if (!elf_res) {
            vga.write("Could not parse payload\n\r");
            vga.write("Error code: ");
            vga.write(tos::itoa(int(force_error(elf_res))).data());
            vga.write("\n\r");
            while (true)
                ;
        }

        runnable_groups.push_back(force_get(
            load_from_elf(force_get(elf_res), *trampoline, *palloc, level0_table)));
    }

    LOG("Done loading");

    int32_t x = 3, y = 42;
    auto params = std::make_tuple(&x, &y);
    auto results = tos::ae::service::calculator::wire_types::add_results{-1};
    auto results2 = tos::ae::service::calculator::wire_types::add_results{-1};

    auto& req1 = tos::ae::submit_req<true>(
        *runnable_groups.front().iface.user_iface, 0, 0, &params, &results);
    auto& req2 = tos::ae::submit_req<true>(
        *runnable_groups.back().iface.user_iface, 0, 0, &params, &results2);

    auto req_task = [&]() -> tos::Task<void> {
        co_await req1;
        co_await req2;
        tos::launch(tos::alloc_stack, [] { LOG("Reqs finished!!"); });
    };

    tos::coro_job j(tos::current_context(), tos::coro::make_pollable(req_task()));
    tos::kern::make_runnable(j);
    tos::this_thread::yield();

    auto syshandler2 = [&](tos::x86_64::syscall_frame& frame) {
        tos::swap_context(*runnable_groups.front().state, self, tos::int_ctx{});
    };
    tos::x86_64::set_syscall_handler(tos::x86_64::syscall_handler_t(syshandler2));

    for (int i = 0; i < 10; ++i) {
        LOG("back", results.ret0(), results2.ret0());
        proc_req_queue(runnable_groups.front().iface);
        tos::this_thread::yield();

        odi([&](auto...) {
            tos::swap_context(self, *runnable_groups.front().state, tos::int_ctx{});
        });
        std::swap(runnable_groups.front(), runnable_groups.back());
    }

    while (true) {
        sem.down();
        LOG("Tick");
    }
}

tos::stack_storage store;
void tos_main() {
    tos::launch(store, thread);
}