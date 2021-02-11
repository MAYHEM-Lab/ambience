#include "common/inet/tcp_ip.hpp"
#include "lwip/init.h"
#include "tos/function_ref.hpp"
#include "tos/lwip/udp.hpp"
#include "tos/memory.hpp"
#include "tos/paging/physical_page_allocator.hpp"
#include "tos/platform.hpp"
#include "tos/self_pointing.hpp"
#include "tos/semaphore.hpp"
#include "tos/thread.hpp"
#include "tos/utility.hpp"
#include "tos/x86_64/exception.hpp"
#include "tos/x86_64/port.hpp"
#include <cstddef>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/lwip/if_adapter.hpp>
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

    tos::x86_64::pic::enable_irq(0);

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

    auto& level0_table = tos::cur_arch::get_current_translation_table();

    dump_table(level0_table);

    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space = tos::align_nearest_up_pow2(
        tos::physical_page_allocator::size_for_pages(1024), 4096);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    auto op_res =
        tos::cur_arch::allocate_region(tos::cur_arch::get_current_translation_table(),
                                       allocator_segment,
                                       tos::user_accessible::no,
                                       nullptr);
    LOG(bool(op_res));

    auto res =
        tos::cur_arch::mark_resident(tos::cur_arch::get_current_translation_table(),
                                     allocator_segment,
                                     tos::memory_types::normal,
                                     vmem_end);
    LOG(bool(res));

    auto palloc = new (vmem_end) tos::physical_page_allocator(1024);
    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, 4096});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    for (int i = 0; i < 5; ++i) {
        auto p = palloc->allocate(1);
        auto ptr = palloc->address_of(*p);
        LOG(i, ptr, palloc->remaining_page_count());
        LOG("Available:", palloc->remaining_page_count());

        auto op_res = tos::cur_arch::allocate_region(
            tos::cur_arch::get_current_translation_table(),
            {{uintptr_t(ptr), 4096}, tos::permissions::read_write},
            tos::user_accessible::no,
            nullptr);
        LOG(bool(op_res));
        LOG("Available:", palloc->remaining_page_count());

        auto res = tos::cur_arch::mark_resident(
            tos::cur_arch::get_current_translation_table(),
            {{uintptr_t(ptr), 4096}, tos::permissions::read_write},
            tos::memory_types::normal,
            ptr);
        LOG(bool(res));
        LOG("Available:", palloc->remaining_page_count());

        *((volatile int*)ptr) = 99;
        res = tos::cur_arch::mark_nonresident(
            tos::cur_arch::get_current_translation_table(),
            {{uintptr_t(ptr), 4096}, tos::permissions::read_write});
        LOG(bool(res));
        LOG("Available:", palloc->remaining_page_count());
    }

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
                    "BAR4",
                    (void*)(uintptr_t)dev.bar4(),
                    "BAR5",
                    (void*)(uintptr_t)dev.bar5(),
                    dev.has_capabilities());

                if (vendor_id == 0x1AF4) {
                    switch (dev.device_id()) {
                    case 0x1001: {
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

                        auto interface =
                            new virtio_net_if(nd,
                                              tos::parse_ipv4_address("10.0.2.15"),
                                              tos::parse_ipv4_address("255.255.255.0"),
                                              tos::parse_ipv4_address("10.0.2.2"));
                        set_default(*interface);
                        interface->up();

                        // uint8_t buf[128]{};
                        // nd->transmit_packet(buf);

                        // while (true) {
                        //     auto packet = nd->take_packet();
                        //     LOG("Got it:", packet);
                        //     nd->return_packet(packet);
                        // }
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

    uint8_t buf[] = "hello world";
    tos::udp_endpoint_t ep{tos::parse_ipv4_address("10.0.2.2"), {1234}};
    sock.send_to(buf, ep);
    sock.send_to(buf, ep);
    sock.send_to(buf, ep);
    sock.send_to(buf, ep);

    tos::semaphore sem{0};

    auto tim_handler = [&](tos::x86_64::exception_frame*, int) { sem.up_isr(); };

    tos::platform::set_irq(
        0, tos::function_ref<void(tos::x86_64::exception_frame*, int)>(tim_handler));

    while (true) {
        sem.down();
    }

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