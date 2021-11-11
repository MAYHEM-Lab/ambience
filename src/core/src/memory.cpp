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
extern uint8_t __bss_map_end;
}

namespace tos::default_segments {
physical_range image() {
    auto beg = reinterpret_cast<uintptr_t>(&__start);
    auto end = reinterpret_cast<uintptr_t>(&__end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}

physical_range data() {
    auto beg = reinterpret_cast<uintptr_t>(&__data_start);
    auto end = reinterpret_cast<uintptr_t>(&__data_end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}
physical_range text() {
    auto beg = reinterpret_cast<uintptr_t>(&__text_start);
    auto end = reinterpret_cast<uintptr_t>(&__text_end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}

physical_range rodata() {
    auto beg = reinterpret_cast<uintptr_t>(&__rodata_start);
    auto end = reinterpret_cast<uintptr_t>(&__rodata_end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}

physical_range bss() {
    auto beg = reinterpret_cast<uintptr_t>(&__bss_start);
    auto end = reinterpret_cast<uintptr_t>(&__bss_end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}

physical_range bss_map() {
    auto beg = reinterpret_cast<uintptr_t>(&__bss_start);
    auto end = reinterpret_cast<uintptr_t>(&__bss_map_end);
    return {.base = physical_address(beg), .size = ptrdiff_t(end - beg)};
}
} // namespace tos::default_segments
