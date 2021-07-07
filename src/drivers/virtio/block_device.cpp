#include <tos/debug/log.hpp>
#include <tos/platform.hpp>
#include <tos/virtio/block_device.hpp>

namespace tos::virtio {
namespace {
constexpr auto dev_base_offset = 0x14;
constexpr auto dev_sector_count_lo_offset = dev_base_offset + 0;
constexpr auto dev_sector_count_hi_offset = dev_base_offset + 4;
constexpr auto dev_sector_size_offset = dev_base_offset + 20;

struct blk_header {
    uint32_t type;
    uint32_t _res;
    uint64_t sector;
};
} // namespace

bool block_device::initialize(tos::physical_page_allocator* palloc) {
    if (!base_initialize(palloc)) {
        return false;
    }

    transport().setup_interrupts(tos::mem_function_ref<&block_device::isr>(*this));
    transport().enable_interrupts();

    LOG("Sector count:", int(number_of_sectors()));
    LOG("Block size:", int(sector_size_bytes()));

    transport().write_byte(status_port_offset, 0xf);
    LOG_TRACE("Device initialized");
    return true;
}

expected<void, int>
block_device::write(uint64_t sector_id, span<const uint8_t> data, size_t offset) {
    if (offset != 0 || data.size() != sector_size_bytes()) {
        return unexpected(-1);
    }

    auto& q = queue_at(0);

    blk_header header{};
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

    q.submit_available(root_idx);

    transport().write_u16(notify_port_offset, 0);

    m_wait_sem.down();

    if (c == 0) {
        return {};
    }
    return unexpected(c);
}

Task<expected<void, int>>
block_device::async_write(uint64_t sector_id, span<const uint8_t> data, size_t offset) {
    if (offset != 0 || data.size() != sector_size_bytes()) {
        co_return unexpected(-1);
    }

    auto& q = queue_at(0);

    blk_header header{};
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

    q.submit_available(root_idx);

    transport().write_u16(notify_port_offset, 0);

    co_await m_wait_sem;

    if (c == 0) {
        co_return tos::expected<void, int>{};
    }
    co_return unexpected(c);
}

expected<void, int>
block_device::read(uint64_t sector_id, span<uint8_t> data, size_t offset) {
    if (offset != 0 || data.size() != sector_size_bytes()) {
        return unexpected(-1);
    }

    auto& q = queue_at(0);

    blk_header header{};
    header.type = 0;
    header.sector = sector_id;

    auto [root_idx, root] = q.alloc();
    auto [data_idx, data_] = q.alloc();
    auto [code_idx, code_] = q.alloc();

    root->addr = reinterpret_cast<uintptr_t>(&header);
    root->len = sizeof header;
    root->flags = queue_flags::next;
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

    q.submit_available(root_idx);

    transport().write_u16(notify_port_offset, 0);

    m_wait_sem.down();

    if (c == 0) {
        return {};
    }
    return unexpected(c);
}

Task<expected<void, int>>
block_device::async_read(uint64_t sector_id, span<uint8_t> data, size_t offset) {

    if (offset != 0 || data.size() != sector_size_bytes()) {
        co_return unexpected(-1);
    }

    auto& q = queue_at(0);

    blk_header header{};
    header.type = 0;
    header.sector = sector_id;

    auto [root_idx, root] = q.alloc();
    auto [data_idx, data_] = q.alloc();
    auto [code_idx, code_] = q.alloc();

    root->addr = reinterpret_cast<uintptr_t>(&header);
    root->len = sizeof header;
    root->flags = queue_flags::next;
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

    q.submit_available(root_idx);

    transport().write_u16(notify_port_offset, 0);

    co_await m_wait_sem;

    if (c == 0) {
        co_return tos::expected<void, int>{};
    }
    co_return unexpected(c);
}

size_t block_device::number_of_sectors() const {
    auto sector_count_hi =
        transport().read_u32((have_msix() ? 4 : 0) + dev_sector_count_hi_offset);
    auto sector_count_lo =
        transport().read_u32((have_msix() ? 4 : 0) + dev_sector_count_lo_offset);
    return (uint64_t(sector_count_hi) << 32) | sector_count_lo;
}

void block_device::isr() {
    auto isr_status = transport().read_byte(interrupt_status_port_offset);
    if (isr_status & 1) {
        m_wait_sem.up_isr();
    }
}

size_t block_device::sector_size_bytes() const {
    return transport().read_u32((have_msix() ? 4 : 0) + dev_sector_size_offset);
}
} // namespace tos::virtio