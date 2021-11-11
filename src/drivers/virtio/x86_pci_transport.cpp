#include "tos/memory.hpp"
#include <deque>
#include <tos/debug/log.hpp>
#include <tos/function_ref.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/common.hpp>
#include <tos/virtio/x86_pci_transport.hpp>
#include <tos/x86_64/apic.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/x86_64/pic.hpp>

extern tos::physical_page_allocator* g_palloc;
namespace tos::virtio {
namespace {
struct msix_cap_data {
    uint32_t reg0, reg1, reg2;

    uint8_t id() const {
        return reg0 & 0xFF;
    }

    uint8_t next_ptr() const {
        return (reg0 >> 8) & 0xFF;
    }

    uint16_t message_control() const {
        return reg0 >> 16;
    }

    int table_size() const {
        return 1 + (message_control() & 0x7FF);
    }

    uint8_t bir() const {
        return reg1 & 0b111;
    }

    uint32_t table_offset() const {
        return reg1 & ~0b111;
    }

    uint8_t pending_bit_bir() const {
        return reg2 & 0b111;
    }

    uint32_t pending_bit_offset() const {
        return reg2 & ~0b111;
    }
};

class impl final : public transport {
public:
    impl(x86_64::pci::device&& dev)
        : m_pci_dev{std::move(dev)}
        , m_irq_handler{mem_function_ref<&impl::dummy_irq>(*this)} {
        for (auto cap = m_pci_dev.capabilities_root(); cap; cap = cap->next()) {
            LOG((void*)(uintptr_t)cap->vendor(),
                bool(cap->next()),
                (void*)(uintptr_t)cap->next()->m_offset,
                (void*)(uintptr_t)cap->len());

            if (cap->vendor() == 0x9) {
                handle_vendor_capability(*cap);
            } else if (cap->vendor() == 0x11) {
                // MSI-X
                handle_msix_capability(*cap);
            } else {
                LOG_WARN("PCI Device has an unknown capability",
                         (void*)(uintptr_t)(cap->vendor()));
            }
        }

        if (m_pci_capability) {
            auto common_bar = m_pci_dev.bars()[m_pci_capability->bar];
            m_bar_base = common_bar & 0xFFFFFFFC;
        } else {
            auto common_bar = m_pci_dev.bars()[0];
            m_bar_base = common_bar & 0xFFFFFFFC;
        }

        allocated_irq_line = force_get(tos::platform::allocate_irq());
    }

    uint8_t read_byte(int offset) override {
        return x86_64::port(m_bar_base + offset).inb();
    }

    void write_byte(int offset, uint8_t data) override {
        x86_64::port(m_bar_base + offset).outb(data);
    }

    uint16_t read_u16(int offset) override {
        return x86_64::port(m_bar_base + offset).inw();
    }

    void write_u16(int offset, uint16_t data) override {
        x86_64::port(m_bar_base + offset).outw(data);
    }

    uint32_t read_u32(int offset) override {
        return x86_64::port(m_bar_base + offset).inl();
    }

    void write_u32(int offset, uint32_t data) override {
        x86_64::port(m_bar_base + offset).outl(data);
    }

    void setup_interrupts(tos::function_ref<void()> handler) override {
        m_irq_handler = handler;
        tos::platform::set_irq(allocated_irq_line,
                               tos::mem_function_ref<&impl::isr>(*this));

        if (has_msix()) {
            msix_cap_data msix_data;
            msix_data.reg0 = m_msix_capability->read_long(0);
            msix_data.reg1 = m_msix_capability->read_long(4);
            msix_data.reg2 = m_msix_capability->read_long(8);

            auto bar = pci_dev().bars()[msix_data.bir()];
            Assert(!(bar & 1));
            auto type = bar & 0b110;
            uintptr_t map_addr = 0;
            if (type == 0) {
                map_addr = bar & ~0b1111;
            } else if (type == 2) {
                auto next_bar = pci_dev().bars()[msix_data.bir() + 1];
                map_addr = (bar & ~0b1111) | (uintptr_t((next_bar & ~0b1111)) << 32);
            } else {
                Assert(false);
            }
            LOG((void*)map_addr);

            auto msix_vec = (uint32_t*)map_addr;

            for (int i = 0; i < msix_data.table_size(); ++i) {
                msix_vec[i * 4 + 0] = 0xfee00000;
                msix_vec[i * 4 + 1] = 0;
                msix_vec[i * 4 + 2] = 32 + allocated_irq_line;
                msix_vec[i * 4 + 3] = 1;
            }
        } else {
            tos::x86_64::ioapic_set_irq(pci_dev().irq_line(), 0, 32 + allocated_irq_line);
        }
    }

    void enable_interrupts() override {
        if (has_msix()) {
            msix_cap_data msix_data;
            msix_data.reg0 = m_msix_capability->read_long(0);
            msix_data.reg1 = m_msix_capability->read_long(4);
            msix_data.reg2 = m_msix_capability->read_long(8);

            auto bar = pci_dev().bars()[msix_data.bir()];
            Assert(!(bar & 1));
            auto type = bar & 0b110;
            uintptr_t map_addr = 0;
            if (type == 0) {
                map_addr = bar & ~0b1111;
            } else if (type == 2) {
                auto next_bar = pci_dev().bars()[msix_data.bir() + 1];
                map_addr = (bar & ~0b1111) | (uintptr_t((next_bar & ~0b1111)) << 32);
            } else {
                Assert(false);
            }
            LOG((void*)map_addr);

            auto msix_vec = (uint32_t*)map_addr;

            for (int i = 0; i < msix_data.table_size(); ++i) {
                msix_vec[i * 4 + 3] = 0;
            }
        } else {
        }
    }

    void disable_interrupts() override {
        if (has_msix()) {
            msix_cap_data msix_data;
            msix_data.reg0 = m_msix_capability->read_long(0);
            msix_data.reg1 = m_msix_capability->read_long(4);
            msix_data.reg2 = m_msix_capability->read_long(8);

            auto bar = pci_dev().bars()[msix_data.bir()];
            Assert(!(bar & 1));
            auto type = bar & 0b110;
            uintptr_t map_addr = 0;
            if (type == 0) {
                map_addr = bar & ~0b1111;
            } else if (type == 2) {
                auto next_bar = pci_dev().bars()[msix_data.bir() + 1];
                map_addr = (bar & ~0b1111) | (uintptr_t((next_bar & ~0b1111)) << 32);
            } else {
                Assert(false);
            }
            LOG((void*)map_addr);

            auto msix_vec = (uint32_t*)map_addr;

            for (int i = 0; i < msix_data.table_size(); ++i) {
                msix_vec[i * 4 + 3] = 1;
            }
        } else {
        }
    }

    void isr(tos::x86_64::exception_frame* f, int num) {
        m_irq_handler();
    }

    tos::x86_64::pci::device& pci_dev() {
        return m_pci_dev;
    }

    bool has_msix() override {
        return m_msix_capability.has_value();
    }

private:
    void dummy_irq() {
    }

    void handle_msix_capability(x86_64::pci::capability& cap) {
        msix_cap_data msix_data;
        msix_data.reg0 = cap.read_long(0);
        msix_data.reg1 = cap.read_long(4);
        msix_data.reg2 = cap.read_long(8);

        LOG_TRACE("MSI-X",
                  (void*)(uintptr_t)msix_data.id(),
                  (void*)(uintptr_t)msix_data.next_ptr(),
                  (void*)(uintptr_t)msix_data.message_control(),
                  (void*)(uintptr_t)msix_data.bir(),
                  (void*)(uintptr_t)msix_data.table_offset(),
                  (void*)(uintptr_t)msix_data.pending_bit_bir(),
                  (void*)(uintptr_t)msix_data.pending_bit_offset(),
                  msix_data.table_size(),
                  (void*)(uintptr_t)msix_data.reg0,
                  (void*)(uintptr_t)msix_data.reg1,
                  (void*)(uintptr_t)msix_data.reg2);

        m_msix_capability = cap;

        auto bar = pci_dev().bars()[msix_data.bir()];
        Assert(!(bar & 1));
        auto type = bar & 0b110;
        uintptr_t map_addr = 0;
        if (type == 0) {
            map_addr = bar & ~0b1111;
        } else if (type == 2) {
            auto next_bar = pci_dev().bars()[msix_data.bir() + 1];
            map_addr = (bar & ~0b1111) | (uintptr_t((next_bar & ~0b1111)) << 32);
        } else {
            Assert(false);
        }
        LOG((void*)map_addr);

        auto segment = tos::virtual_segment{
            .range = {.base = virtual_address(map_addr), .size = x86_64::page_size_bytes},
            .perms = permissions::read_write};
        auto& root = x86_64::get_current_translation_table();
        auto res = x86_64::map_region(root,
                                      segment,
                                      user_accessible::no,
                                      memory_types::device,
                                      g_palloc,
                                      physical_address(map_addr));
        Assert(res);

        auto msix_vec = (uint32_t*)map_addr;

        for (int i = 0; i < msix_data.table_size(); ++i) {
            msix_vec[i * 4 + 0] = 0xfee00000;
            msix_vec[i * 4 + 1] = 0;
            msix_vec[i * 4 + 2] = 32 + allocated_irq_line;
            msix_vec[i * 4 + 3] = 1;
        }

        m_msix_capability->write_word(2, m_msix_capability->read_word(2) | 0x8000);
        LOG((void*)(uintptr_t)m_msix_capability->read_long(0));
    }

    void handle_vendor_capability(x86_64::pci::capability& cap) {
        // Virtio vendor capability
        LOG_TRACE("Type:",
                  (void*)(uintptr_t)cap.read_byte(3),
                  "BAR:",
                  (void*)(uintptr_t)cap.read_byte(4),
                  "Offset:",
                  (void*)(uintptr_t)cap.read_long(8),
                  "Length:",
                  (void*)(uintptr_t)cap.read_long(12));

        capability_data data;
        data.type = static_cast<capability_type>(cap.read_byte(3));
        data.bar = cap.read_byte(4);
        data.offset = cap.read_long(8);
        data.length = cap.read_long(12);
        m_capabilities.emplace_back(data);

        if (data.type == capability_type::pci) {
            m_pci_capability = &m_capabilities.back();
        }
    }

    struct capability_data {
        capability_type type;
        uint8_t bar;
        uint32_t offset;
        uint32_t length;
    };

    capability_data* m_pci_capability = nullptr;
    std::optional<x86_64::pci::capability> m_msix_capability;
    std::deque<capability_data> m_capabilities;

    x86_64::pci::device m_pci_dev;
    uint32_t m_bar_base;

    tos::function_ref<void()> m_irq_handler;

    int allocated_irq_line;
};
} // namespace

std::unique_ptr<transport> make_x86_pci_transport(x86_64::pci::device&& dev) {
    return std::make_unique<impl>(std::move(dev));
}
} // namespace tos::virtio