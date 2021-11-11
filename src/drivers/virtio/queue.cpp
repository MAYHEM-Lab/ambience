#include "tos/debug/panic.hpp"
#include "tos/memory.hpp"
#include "tos/utility.hpp"
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>
#include <tos/virtio/queue.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::virtio {
queue::queue(uint16_t sz, tos::physical_page_allocator& palloc)
    : size(sz) {
    auto descriptor_sz = sizeof(queue_descriptor) * sz;
    auto available_sz = sizeof(queue_available) + sizeof(uint16_t) * sz;

    auto desc_avail_sz = tos::align_nearest_up_pow2(descriptor_sz + available_sz,
                                                    tos::cur_arch::page_size_bytes);
    LOG(int(desc_avail_sz), int(descriptor_sz + available_sz));

    auto used_sz = sizeof(queue_used) + sizeof(queue_used_elem) * sz;
    auto total_sz = desc_avail_sz +
                    tos::align_nearest_up_pow2(used_sz, tos::cur_arch::page_size_bytes);
    auto pages = total_sz / palloc.page_size();
    LOG("Need", int(total_sz), "bytes", int(pages), "pages");

    auto pages_ptr = palloc.allocate(pages);

    if (!pages_ptr) {
        LOG_ERROR("Could not allocate pages!");
        tos::debug::panic("Could not allocate pages!");
    }

    auto buf = palloc.address_of(*pages_ptr);

    auto map_res = tos::cur_arch::map_region(
        tos::cur_arch::get_current_translation_table(),
        identity_map(physical_segment{physical_range{buf, ptrdiff_t(total_sz)},
                                      tos::permissions::read_write}),
        tos::user_accessible::no,
        tos::memory_types::normal,
        &palloc,
        buf);
    LOG(bool(map_res));

    std::fill((char*)buf.direct_mapped(), (char*)buf.direct_mapped() + total_sz, 0);
    LOG("Buffer:", buf.direct_mapped());

    descriptors_base = reinterpret_cast<queue_descriptor*>(buf.direct_mapped());
    available_base =
        reinterpret_cast<queue_available*>((char*)buf.direct_mapped() + descriptor_sz);
    used_base = reinterpret_cast<volatile queue_used*>((char*)buf.direct_mapped() +
                                                       desc_avail_sz);

    LOG(descriptors_base, available_base, (void*)used_base);
}

void queue::dump_descriptor(int index) {
    while (true) {
        auto& desc = descriptors()[index];

        tos::debug::log("Descriptor",
                        index,
                        "at",
                        (void*)desc.addr,
                        "len",
                        desc.len,
                        "flags",
                        desc.flags,
                        "next",
                        desc.next);

        if (!(desc.flags & queue_flags::next)) {
            break;
        }
        index = desc.next;
    }
}
} // namespace tos::virtio