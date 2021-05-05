#pragma once

namespace tos::virtio {
enum capability_type
{
    common = 1,
    notify = 2,
    isr = 3,
    device = 4,
    pci = 5
};

constexpr auto dev_features_port_offset = 0x0;
constexpr auto drv_features_port_offset = 0x4;
constexpr auto queue_addr_port_offset = 0x8;
constexpr auto queue_size_port_offset = 0xc;
constexpr auto queue_sel_port_offset = 0xe;
constexpr auto notify_port_offset = 0x10;
constexpr auto status_port_offset = 0x12;
constexpr auto interrupt_status_port_offset = 0x13;
constexpr auto config_msix_vector_offset = 0x14;
constexpr auto queue_msix_vector_offset = 0x16;
} // namespace tos::virtio