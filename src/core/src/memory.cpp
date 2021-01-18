#include <new>
#include <tos/memory.hpp>

extern "C" {
extern uint8_t __start;
extern uint8_t __end;

extern uint8_t __data_start;
extern uint8_t __data_end;

extern uint8_t __text_start;
extern uint8_t __text_end;

extern uint8_t __rodata_start;
extern uint8_t __rodata_end;

extern uint8_t __bss_start;
extern uint8_t __bss_end;
}

namespace tos::default_segments {
memory_range image() {
    auto beg = reinterpret_cast<uintptr_t>(&__start);
    auto end = reinterpret_cast<uintptr_t>(&__end);
    return {.base = beg, .size = ptrdiff_t(end - beg)};
}

memory_range data() {
    auto beg = reinterpret_cast<uintptr_t>(&__data_start);
    auto end = reinterpret_cast<uintptr_t>(&__data_end);
    return {.base = beg, .size = ptrdiff_t(end - beg)};
}
memory_range text() {
    auto beg = reinterpret_cast<uintptr_t>(&__text_start);
    auto end = reinterpret_cast<uintptr_t>(&__text_end);
    return {.base = beg, .size = ptrdiff_t(end - beg)};
}

memory_range rodata() {
    auto beg = reinterpret_cast<uintptr_t>(&__rodata_start);
    auto end = reinterpret_cast<uintptr_t>(&__rodata_end);
    return {.base = beg, .size = ptrdiff_t(end - beg)};
}

memory_range bss() {
    auto beg = reinterpret_cast<uintptr_t>(&__bss_start);
    auto end = reinterpret_cast<uintptr_t>(&__bss_end);
    return {.base = beg, .size = ptrdiff_t(end - beg)};
}
} // namespace tos::default_segments

namespace tos {
std::unique_ptr<mapping>
physical_memory_backing::create_mapping(const segment& vm_segment,
                                        const memory_range& obj_range) {
    if (!contains(m_seg.range, obj_range)) {
        return nullptr;
    }

    if ((int(vm_segment.perms) & int(m_seg.perms)) != int(vm_segment.perms)) {
        return nullptr;
    }

    auto res = std::make_unique<mapping>();
    res->obj = intrusive_ptr<backing_object>(this);
    res->vm_segment = vm_segment;
    res->obj_range = obj_range;
    res->mem_type = m_type;
    return res;
}

physical_page_allocator::physical_page_allocator(size_t num_pages)
    : m_num_pages{num_pages}, m_remaining{num_pages} {
    for (auto& page : get_table()) {
        new (&page) physical_page();
    }
    memory_range this_obj;
    this_obj.base = reinterpret_cast<uintptr_t>(this);
    this_obj.size = sizeof *this + get_table().size_bytes();
    mark_unavailable(this_obj);
}

intrusive_ptr<physical_page> physical_page_allocator::allocate(int count, int align) {
    if (count != 1 || align != 1)
        return nullptr;
    for (auto& page : get_table()) {
        if (page.free()) {
            m_remaining -= 1;
            return intrusive_ptr<physical_page> (&page);
        }
    }
    return nullptr;
}

void* physical_page_allocator::address_of(const physical_page& page) const {
    return reinterpret_cast<void*>(page_num(page) * 4096);
}

int physical_page_allocator::page_num(const physical_page& page) const {
    return std::distance(get_table().data(), &page);
}

void physical_page_allocator::mark_unavailable(const memory_range& len) {
    auto begin_num = align_nearest_down_pow2(len.base, 4096) / 4096;
    auto end_num = align_nearest_up_pow2(len.end(), 4096) / 4096;
    begin_num = std::min<int>(m_num_pages, begin_num);
    end_num = std::min<int>(m_num_pages, end_num);
    for (int i = begin_num; i < end_num; ++i) {
        m_remaining -= 1;
        intrusive_ref(&get_table()[i]);
    }
}

physical_page* physical_page_allocator::info(void* ptr) {
    return info(reinterpret_cast<uintptr_t>(ptr) / 4096);
}

physical_page* physical_page_allocator::info(int32_t page_num) {
    if (page_num >= m_num_pages) {
        return nullptr;
    }

    return &get_table()[page_num];
}
} // namespace tos