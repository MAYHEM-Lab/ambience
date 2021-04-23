#include <tos/debug/log.hpp>
#include <tos/function_ref.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/common.hpp>
#include <tos/virtio/x86_pci_transport.hpp>
#include <tos/x86_64/pci.hpp>
#include <tos/x86_64/pic.hpp>

namespace tos::virtio {
namespace {
class impl : public transport {
public:
    impl(x86_64::pci::device&& dev)
        : m_pci_dev{std::move(dev)}
        , m_irq_handler{mem_function_ref<&impl::dummy_irq>(*this)} {
        for (auto cap = m_pci_dev.capabilities_root(); cap; cap = cap->next()) {
            LOG((void*)(uintptr_t)cap->vendor(),
                (void*)(uintptr_t)cap->next()->m_offset,
                (void*)(uintptr_t)cap->len());

            if (cap->vendor() == 0x9) {
                handle_capability(*cap);
            }
        }

        if (m_pci_capability) {
            auto common_bar = m_pci_dev.bars()[m_pci_capability->bar];
            m_bar_base = common_bar & 0xFFFFFFFC;
        } else {
            auto common_bar = m_pci_dev.bars()[0];
            m_bar_base = common_bar & 0xFFFFFFFC;
        }
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

    void enable_interrupts(tos::function_ref<void()> handler) override {
        m_irq_handler = handler;
        tos::platform::set_irq(pci_dev().irq_line(),
                               tos::mem_function_ref<&impl::isr>(*this));
        tos::x86_64::pic::enable_irq(pci_dev().irq_line());
    }

    void disable_interrupts() override {
    }

    void isr(tos::x86_64::exception_frame* f, int num) {
        m_irq_handler();
    }

    tos::x86_64::pci::device& pci_dev() {
        return m_pci_dev;
    }

private:
    void dummy_irq() {
    }

    void handle_capability(x86_64::pci::capability& cap) {
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
    std::vector<capability_data> m_capabilities;

    x86_64::pci::device m_pci_dev;
    uint32_t m_bar_base;

    tos::function_ref<void()> m_irq_handler;
};
} // namespace

std::unique_ptr<transport> make_x86_pci_transport(x86_64::pci::device&& dev) {
    return std::make_unique<impl>(std::move(dev));
}
} // namespace tos::virtio