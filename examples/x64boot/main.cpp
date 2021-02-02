#include "tos/x86_64/exception.hpp"
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/x86_64/mmu.hpp>
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

namespace tos::pci {
enum classes
{
    unclassified = 0,
    storage = 1,
    network = 2,
    display = 3,
    multimedia = 4,
    memory = 5
};
}

namespace tos::x86_64::pci {
namespace detail {
constexpr port address_port{0xcf8};
constexpr port data_port{0xcfc};

uint32_t config_read_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto lbus = static_cast<uint32_t>(bus);
    auto lslot = static_cast<uint32_t>(slot);
    auto lfunc = static_cast<uint32_t>(func);

    auto address = lbus << 16 | lslot << 11 | lfunc << 8 | (offset & 0xfc) | 0x80000000;
    address_port.outl(address);

    auto data = data_port.inl();
    return data;
}

void config_write_raw(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    auto lbus = static_cast<uint32_t>(bus);
    auto lslot = static_cast<uint32_t>(slot);
    auto lfunc = static_cast<uint32_t>(func);

    auto address = lbus << 16 | lslot << 11 | lfunc << 8 | (offset & 0xfc) | 0x80000000;
    address_port.outl(address);

    data_port.outl(value);
}

uint32_t config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    return data;
}

uint16_t config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b10;
    auto bits = in_offset * 8;

    return (data >> bits) & 0xffff;
}

uint8_t config_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    auto data = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b11;
    auto bits = in_offset * 8;

    return (data >> bits) & 0xff;
}

void config_write_dword(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    return config_write_raw(bus, slot, func, offset, value);
}

void config_write_byte(
    uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value) {
    auto qword = config_read_raw(bus, slot, func, offset);
    auto in_offset = offset & 0b11;
    auto bits = in_offset * 8;

    qword &= ~(0xff << bits);
    qword |= static_cast<uint32_t>(value) << bits;

    config_write_raw(bus, slot, func, offset, qword);
}
} // namespace detail

uint16_t get_vendor(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 0);
}

uint16_t get_dev_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 2);
}

uint8_t get_class_code(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xa + 1);
}

uint8_t get_subclass(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xa);
}

uint16_t get_subsys_id(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_word(bus, slot, func, 0x2c + 2);
}

uint8_t get_header_type(uint8_t bus, uint8_t slot, uint8_t func) {
    return detail::config_read_byte(bus, slot, func, 0xc + 2);
}

class device;
struct capability {

    uint8_t vendor() const;
    std::optional<capability> next() const;
    uint8_t len() const;

    uint8_t read_byte(uint8_t offset) const;
    uint16_t read_word(uint8_t offset) const;
    uint32_t read_long(uint8_t offset) const;

    const device* m_dev;
    uint8_t m_offset;
};

class device {
public:
    device(uint8_t bus, uint8_t slot, uint8_t func)
        : m_bus{bus}
        , m_slot{slot}
        , m_func{func} {
    }

    tos::pci::classes class_code() const {
        return tos::pci::classes(get_class_code(m_bus, m_slot, m_func));
    }

    bool has_capabilities() const {
        return status() & 1 << 4;
    }

    uint16_t vendor() const {
        return get_vendor(m_bus, m_slot, m_func);
    }

    uint16_t device_id() const {
        return get_dev_id(m_bus, m_slot, m_func);
    }

    uint16_t status() const {
        return detail::config_read_word(m_bus, m_slot, m_func, 0x6);
    }

    uint32_t bar0() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x10);
    }

    uint32_t bar1() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x14);
    }

    uint32_t bar2() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x18);
    }

    uint32_t bar3() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x1C);
    }

    uint32_t bar4() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x20);
    }

    uint32_t bar5() const {
        return detail::config_read_dword(m_bus, m_slot, m_func, 0x24);
    }

    std::array<uint32_t, 6> bars() const {
        return {bar0(), bar1(), bar2(), bar3(), bar4(), bar5()};
    }

    std::optional<capability> capabilities_root() const {
        if (has_capabilities()) {
            return capability{
                this,
                static_cast<uint8_t>(
                    detail::config_read_byte(m_bus, m_slot, m_func, 0x34) & 0xFC)};
        }
        return {};
    }

    uint8_t irq_line() const {
        return detail::config_read_byte(m_bus, m_slot, m_func, 0x3c);
    }

    void irq_line(uint8_t line) {
        detail::config_write_byte(m_bus, m_slot, m_func, 0x3c, line);
    }

    template<class Fn, class... Args>
    auto call_fn(const Fn& fn, Args&&... args) const {
        return std::invoke(fn, m_bus, m_slot, m_func, std::forward<Args>(args)...);
    }

private:
    uint8_t m_bus, m_slot, m_func;
};

uint8_t capability::vendor() const {
    return m_dev->call_fn(detail::config_read_byte, m_offset);
}

std::optional<capability> capability::next() const {
    auto ptr = m_dev->call_fn(detail::config_read_byte, m_offset + 1);
    if (ptr != 0) {
        return capability{m_dev, ptr};
    }
    return std::optional<capability>();
}

uint8_t capability::len() const {
    return m_dev->call_fn(detail::config_read_byte, m_offset + 2);
}

uint8_t capability::read_byte(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_byte, m_offset + offset);
}
uint16_t capability::read_word(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_word, m_offset + offset);
}
uint32_t capability::read_long(uint8_t offset) const {
    return m_dev->call_fn(detail::config_read_dword, m_offset + offset);
}
} // namespace tos::x86_64::pci

extern "C" {
void abort() {
    LOG_ERROR("Abort called");
    while (true)
        ;
}
}

namespace tos::virtio {
enum pci_capability_type
{
    common = 1,
    notify = 2,
    isr = 3,
    device = 4,
    pci = 5
};

enum queue_flags : uint16_t
{
    next = 1 << 0,
    write = 1 << 1,
    indirect = 1 << 2,
    available = 1 << 7,
    used = 1 << 15
};

struct queue_descriptor {
    // Physical address
    uint64_t addr{};
    uint32_t len{};
    queue_flags flags{};
    uint16_t next{};
};

static_assert(sizeof(queue_descriptor) == 16);

struct queue_available {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[]; // stores ids
    //    uint16_t used_event;
};

static_assert(sizeof(queue_available) == 4);

struct queue_used_elem {
    uint32_t id;
    uint32_t len;
};

struct queue_used {
    uint16_t flags;
    uint16_t index;
    queue_used_elem ring[];
    // uint16_t avail_event;
};

alignas(4096) uint8_t pages[4096 * 4];

struct queue {
    uint16_t size;

    queue_descriptor* descriptors_base;
    queue_available* available_base;
    volatile queue_used* used_base;

    uint16_t next_buffer = 0;

    std::pair<int, queue_descriptor*> alloc() {
        auto idx = next_buffer % size;
        ++next_buffer;
        return {idx, &descriptors()[idx]};
    }
    
    tos::span<const queue_descriptor> descriptors() const {
        return {descriptors_base, size};
    }

    tos::span<queue_descriptor> descriptors() {
        return {descriptors_base, size};
    }

    explicit queue(uint16_t sz)
        : size(sz) {
        auto descriptor_sz = sizeof(queue_descriptor) * sz;
        auto available_sz = sizeof(queue_available) + sizeof(uint16_t) * sz;

        auto desc_avail_sz =
            tos::align_nearest_up_pow2(descriptor_sz + available_sz, 4096);
        LOG(int(desc_avail_sz), int(descriptor_sz + available_sz));

        auto used_sz = sizeof(queue_used) + sizeof(queue_used_elem) * sz;
        auto total_sz = desc_avail_sz + used_sz;
        LOG("Need", int(total_sz), "bytes");

        void* buf = &pages[0];
        std::fill((char*)buf, (char*)buf + 4096 * 4, 0);
        LOG("Buffer:", buf);

        //        for (int i = 0; i < sz; ++i) {
        //            available_base->ring[i] = 0xFFFF;
        //            used_base->ring[i].id = 0xFFFF;
        //        }

        descriptors_base = reinterpret_cast<queue_descriptor*>(buf);
        available_base = reinterpret_cast<queue_available*>((char*)buf + descriptor_sz);
        used_base = reinterpret_cast<volatile queue_used*>((char*)buf + desc_avail_sz);
        LOG(available_base->index);

        LOG(descriptors_base, available_base, (void*)used_base);
    }
};

class dev {
public:
    explicit dev(x86_64::pci::device&& pci_dev)
        : m_pci_dev{std::move(pci_dev)} {
        for (auto cap = m_pci_dev.capabilities_root(); cap; cap = cap->next()) {
            LOG((void*)cap->vendor(), (void*)cap->next()->m_offset, (void*)cap->len());

            if (cap->vendor() == 0x9) {
                handle_capability(*cap);
            }
        }
    }

    virtual void initialize() = 0;

protected:
    uint32_t bar_base() const {
        auto common_bar = m_pci_dev.bars()[m_pci->bar];
        auto bar_base = common_bar & 0xFFFFFFFC;
        return bar_base;
    }

    void base_initialize() {
        auto bar_base = this->bar_base();
        LOG("Bar base", (void*)bar_base);
        auto status_port = x86_64::port(bar_base + 0x12);

        // The spec says a device MUST initialize this register to 0.
        // However, not setting this register prevents proper initialization for some
        // reason.
        status_port.outb(0);
        status_port.outb(0x1);
        status_port.outb(0x3);

        auto dev_features_port = x86_64::port(bar_base + 0x0);
        auto features = dev_features_port.inl();
        LOG("Features:", (void*)features);

        auto driver_features_port = x86_64::port(bar_base + 0x4);
        driver_features_port.outl(negotiate(features));

        status_port.outb(0xB);

        auto resp = status_port.inb();
        LOG("Status:", resp);
        if ((resp & 0x8) == 0) {
            LOG_ERROR("Feature negotiation failed");
            return;
        }
        LOG_INFO("Features negotiated");

        auto queue_sel_port = x86_64::port(bar_base + 0xe);
        auto queue_sz_port = x86_64::port(bar_base + 0xc);
        auto queue_base_port = x86_64::port(bar_base + 0x8);

        for (int i = 0; i < 16; ++i) {
            queue_sel_port.outw(i);
            auto sz = queue_sz_port.inw();
            if (sz == 0) {
                continue;
            }
            LOG("Queue", i, "size", sz);
            m_queues.emplace_back(sz);
            auto queue_base =
                reinterpret_cast<uintptr_t>(m_queues.back().descriptors_base) / 4096;
            queue_base_port.outl(queue_base);
            LOG(int(queue_base));
        }

        status_port.outb(0xf);
        LOG("Device initialized");
        LOG((void*)status_port.inb());
    }

    virtual uint32_t negotiate(uint32_t features) = 0;

    queue& queue_at(int idx) {
        return m_queues[idx];
    }

    tos::x86_64::pci::device& pci_dev() {
        return m_pci_dev;
    }

private:
    struct capability_data {
        pci_capability_type type;
        uint8_t bar;
        uint32_t offset;
        uint32_t length;
    };

    void handle_capability(x86_64::pci::capability& cap) {
        // Virtio vendor capability
        LOG("Type:",
            (void*)cap.read_byte(3),
            "BAR:",
            (void*)cap.read_byte(4),
            "Offset:",
            (void*)cap.read_long(8),
            "Length:",
            (void*)cap.read_long(12));

        capability_data data;
        data.type = static_cast<pci_capability_type>(cap.read_byte(3));
        data.bar = cap.read_byte(4);
        data.offset = cap.read_long(8);
        data.length = cap.read_long(12);
        m_capabilities.emplace_back(data);

        if (data.type == pci_capability_type::pci) {
            m_pci = &m_capabilities.back();
        }
    }

    capability_data* m_common;
    capability_data* m_pci;
    std::vector<capability_data> m_capabilities;
    x86_64::pci::device m_pci_dev;
    std::vector<queue> m_queues;
};

semaphore pci_irq11_sem{0};

class block_device : public dev {
public:
    using dev::dev;

    int write();
    /**
     * Returns the size of each flash sector/page in bytes.
     */
    size_t sector_size_bytes() const {
        auto bar_base = this->bar_base();

        auto block_size_port = x86_64::port(bar_base + 0x18 + 16);
        return block_size_port.inl();
    }

    /**
     * Number of sectors in the flash.
     */
    size_t number_of_sectors() const {
        auto bar_base = this->bar_base();
        auto sector_count_port_hi = x86_64::port(bar_base + 0x18);
        auto sector_count_port_lo = x86_64::port(bar_base + 0x14);
        return (uint64_t(sector_count_port_hi.inl()) << 32) | sector_count_port_lo.inl();
    }

    expected<void, int> read(uint64_t sector_id, span<uint8_t> data, size_t offset) {
        if (offset != 0 || data.size() != sector_size_bytes()) {
            return unexpected(-1);
        }

        auto& q = queue_at(0);

        req_header header{};
        header.type = 0;
        header.sector = 0;

        auto [root_idx, root] = q.alloc();
        auto [data_idx, data_] = q.alloc();
        auto [code_idx, code_] = q.alloc();

        root->addr = reinterpret_cast<uintptr_t>(&header);
        root->len = sizeof header;
        root->next = data_idx;
        root->flags = queue_flags(queue_flags::next);

        data_->addr = reinterpret_cast<uintptr_t>(data.data());
        data_->len = data.size();
        data_->next = code_idx;
        data_->flags = queue_flags(queue_flags::next | queue_flags::write);

        char c;
        code_->addr = reinterpret_cast<uintptr_t>(&c);
        code_->len = sizeof c;
        code_->flags = queue_flags::write;
        code_->next = {};

        LOG(int(root_idx));
        q.available_base->ring[0] = root_idx;

        q.available_base->index++;

        LOG(q.used_base->index,
            q.used_base->ring[0].id,
            q.used_base->ring[0].len,
            int(c),
            (void*)data[0]);


        auto bar_base = this->bar_base();

        auto notify_port = x86_64::port(bar_base + 0x10);
        notify_port.outw(0);

        pci_irq11_sem.down();

        LOG("out", q.used_base->index,
            q.used_base->ring[0].id,
            q.used_base->ring[0].len,
            int(c),
            (void*)data[0]);
        if (c == 0) {
            return {};
        }
        return unexpected(c);
    }

    void initialize() override {
        base_initialize();

        tos::platform::set_irq(
            pci_dev().irq_line(),
            tos::free_function_ref(+[](tos::x86_64::exception_frame* f, int num) {
//                LOG("PCI IRQ!", num);

                tos::virtio::pci_irq11_sem.up_isr();
            }));
        tos::x86_64::pic::enable_irq(pci_dev().irq_line());

        LOG("Sector count:", int(number_of_sectors()));
        LOG("Block size:", int(sector_size_bytes()));
    }

    void isr();

protected:
private:
    struct req_header {
        uint32_t type;
        uint32_t _res;
        uint64_t sector;
    };

    uint32_t negotiate(uint32_t features) override {
        LOG("RO:", bool(features & feature_read_only));
        LOG("Blksz:", bool(features & feature_blk_size));
        LOG("Topo:", bool(features & feature_topology));
        LOG("Ring evt idx:", bool(features & ring_event_idx));
        return features & ~(feature_topology | ring_event_idx);
    }

    static constexpr uint32_t feature_read_only = 1 << 5;
    static constexpr uint32_t feature_blk_size = 1 << 6;
    static constexpr uint32_t feature_topology = 1 << 10;
    static constexpr uint32_t ring_event_idx = 1 << 29;
};
} // namespace tos::virtio

extern "C" {
void irq10_handler(tos::x86_64::exception_frame* f) {
    tos::x86_64::port(0x20).outb(0x20);
}
void irq11_handler(tos::x86_64::exception_frame* f) {
    tos::virtio::pci_irq11_sem.up_isr();
    tos::x86_64::port(0x20).outb(0x20);
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

                if (vendor_id == 0x1AF4 && dev.device_id() == 0x1001) {
                    LOG("Virtio block device");
                    auto blk_dev = new tos::virtio::block_device(std::move(dev));
                    blk_dev->initialize();
                    uint8_t buf[512];
                    blk_dev->read(0, buf, 0);
                    blk_dev->read(0, buf, 0);
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