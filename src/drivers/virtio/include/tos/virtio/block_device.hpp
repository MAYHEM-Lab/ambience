#pragma once

#include <tos/expected.hpp>
#include <tos/semaphore.hpp>
#include <tos/task.hpp>
#include <tos/virtio/device.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::virtio {
class block_device : public device {
public:
    using device::device;

    /**
     * Returns the size of each flash sector/page in bytes.
     */
    size_t sector_size_bytes() const;

    /**
     * Number of sectors in the flash.
     */
    size_t number_of_sectors() const;

    expected<void, int>
    write(uint64_t sector_id, span<const uint8_t> data, size_t offset);

    Task<expected<void, int>>
    async_write(uint64_t sector_id, span<const uint8_t> data, size_t offset);

    expected<void, int> read(uint64_t sector_id, span<uint8_t> data, size_t offset);

    Task<expected<void, int>>
    async_read(uint64_t sector_id, span<uint8_t> data, size_t offset);

    bool initialize(tos::physical_page_allocator* palloc) override;

    void isr();

protected:
private:
    uint32_t negotiate(uint32_t features) override {
        return features & ~(feature_topology | ring_event_idx);
    }

    tos::semaphore m_wait_sem{0};

    static constexpr uint32_t feature_read_only = 1 << 5;
    static constexpr uint32_t feature_blk_size = 1 << 6;
    static constexpr uint32_t feature_topology = 1 << 10;
};
} // namespace tos::virtio