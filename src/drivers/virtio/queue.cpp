#include <tos/debug/log.hpp>
#include <tos/virtio/queue.hpp>

namespace tos::virtio {
queue::queue(uint16_t sz, tos::physical_page_allocator& palloc)
    : size(sz) {
    auto descriptor_sz = sizeof(queue_descriptor) * sz;
    auto available_sz = sizeof(queue_available) + sizeof(uint16_t) * sz;

    auto desc_avail_sz = tos::align_nearest_up_pow2(descriptor_sz + available_sz, 4096);
    LOG(int(desc_avail_sz), int(descriptor_sz + available_sz));

    auto used_sz = sizeof(queue_used) + sizeof(queue_used_elem) * sz;
    auto total_sz = desc_avail_sz + used_sz;
    LOG("Need", int(total_sz), "bytes");

    auto pages_ptr = palloc.allocate(total_sz / palloc.page_size());
    auto buf = palloc.address_of(*pages_ptr);
    // Map pages to current address space
    std::fill((char*)buf, (char*)buf + total_sz, 0);
    LOG("Buffer:", buf);

    descriptors_base = reinterpret_cast<queue_descriptor*>(buf);
    available_base = reinterpret_cast<queue_available*>((char*)buf + descriptor_sz);
    used_base = reinterpret_cast<volatile queue_used*>((char*)buf + desc_avail_sz);

    LOG(descriptors_base, available_base, (void*)used_base);
}
} // namespace tos::virtio