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
} // namespace tos