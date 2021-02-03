#include <tos/debug/log.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/block_device.hpp>
#include <tos/x86_64/pic.hpp>

namespace tos::virtio {
namespace {
constexpr auto sector_count_lo_offset = 0x14;
constexpr auto sector_count_hi_offset = 0x18;
}

bool block_device::initialize(tos::physical_page_allocator* palloc) {
    if (!base_initialize(palloc)) {
        return false;
    }

    tos::platform::set_irq(pci_dev().irq_line(),
                           tos::mem_function_ref<&block_device::isr>(*this));
    tos::x86_64::pic::enable_irq(pci_dev().irq_line());

    LOG("Sector count:", int(number_of_sectors()));
    LOG("Block size:", int(sector_size_bytes()));

    auto bar_base = this->bar_base();
    auto status_port = x86_64::port(bar_base + status_port_offset);
    status_port.outb(0xf);
    LOG_TRACE("Device initialized");
    return true;
}

expected<void, int>
block_device::write(uint64_t sector_id, span<const uint8_t> data, size_t offset) {
    if (offset != 0 || data.size() != sector_size_bytes()) {
        return unexpected(-1);
    }

    auto& q = queue_at(0);

    req_header header{};
    header.type = 1;
    header.sector = sector_id;

    auto [root_idx, root] = q.alloc();
    auto [data_idx, data_] = q.alloc();
    auto [code_idx, code_] = q.alloc();

    root->addr = reinterpret_cast<uintptr_t>(&header);
    root->len = sizeof header;
    root->flags = queue_flags(queue_flags::next);
    root->next = data_idx;

    data_->addr = reinterpret_cast<uintptr_t>(data.data());
    data_->len = data.size();
    data_->flags = queue_flags(queue_flags::next);
    data_->next = code_idx;

    char c;
    code_->addr = reinterpret_cast<uintptr_t>(&c);
    code_->len = sizeof c;
    code_->flags = queue_flags::write;
    code_->next = {};

    auto avail_idx = q.available_base->index % q.size;
    LOG(int(root_idx), avail_idx);

    q.available_base->ring[avail_idx] = root_idx;

    q.available_base->index++;

    LOG(q.used_base->index,
        q.used_base->ring[avail_idx].id,
        q.used_base->ring[avail_idx].len,
        int(c),
        (void*)data[0]);


    auto bar_base = this->bar_base();

    auto notify_port = x86_64::port(bar_base + notify_port_offset);
    notify_port.outw(0);

    m_wait_sem.down();

    LOG("out",
        q.used_base->index,
        q.used_base->ring[avail_idx].id,
        q.used_base->ring[avail_idx].len,
        int(c),
        (void*)data[0]);
    if (c == 0) {
        return {};
    }
    return unexpected(c);
}

expected<void, int>
block_device::read(uint64_t sector_id, span<uint8_t> data, size_t offset) {
    if (offset != 0 || data.size() != sector_size_bytes()) {
        return unexpected(-1);
    }

    auto& q = queue_at(0);

    req_header header{};
    header.type = 0;
    header.sector = sector_id;

    auto [root_idx, root] = q.alloc();
    auto [data_idx, data_] = q.alloc();
    auto [code_idx, code_] = q.alloc();

    root->addr = reinterpret_cast<uintptr_t>(&header);
    root->len = sizeof header;
    root->flags = queue_flags(queue_flags::next);
    root->next = data_idx;

    data_->addr = reinterpret_cast<uintptr_t>(data.data());
    data_->len = data.size();
    data_->flags = queue_flags(queue_flags::next | queue_flags::write);
    data_->next = code_idx;

    char c;
    code_->addr = reinterpret_cast<uintptr_t>(&c);
    code_->len = sizeof c;
    code_->flags = queue_flags::write;
    code_->next = {};

    auto avail_idx = q.available_base->index % q.size;
    LOG(int(root_idx), avail_idx);

    q.available_base->ring[avail_idx] = root_idx;

    q.available_base->index++;

    LOG(q.used_base->index,
        q.used_base->ring[avail_idx].id,
        q.used_base->ring[avail_idx].len,
        int(c),
        (void*)data[0]);


    auto bar_base = this->bar_base();

    auto notify_port = x86_64::port(bar_base + notify_port_offset);
    notify_port.outw(0);

    m_wait_sem.down();

    LOG("out",
        q.used_base->index,
        q.used_base->ring[avail_idx].id,
        q.used_base->ring[avail_idx].len,
        int(c),
        (void*)data[0]);
    if (c == 0) {
        return {};
    }
    return unexpected(c);
}

size_t block_device::number_of_sectors() const {
    auto bar_base = this->bar_base();
    auto sector_count_port_hi = x86_64::port(bar_base + sector_count_hi_offset);
    auto sector_count_port_lo = x86_64::port(bar_base + sector_count_lo_offset);
    return (uint64_t(sector_count_port_hi.inl()) << 32) | sector_count_port_lo.inl();
}
} // namespace tos::virtio