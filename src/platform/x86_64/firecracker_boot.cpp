#include <tos/compiler.hpp>
#include <tos/stack_storage.hpp>
#include <tos/x86_64/spmanip.hpp>

namespace {
NO_ZERO tos::stack_storage main_stack{};
}
//
// Instead of defining full boot_params and setup_header structs as in
// Linux source code, we define only handful of offsets pointing the fields
// we need to read from there. For details please this chunk of Linux code -
// https://github.com/torvalds/linux/blob/b6839ef26e549de68c10359d45163b0cfb031183/arch/x86/include/uapi/asm/bootparam.h#L151-L198
#define LINUX_KERNEL_BOOT_FLAG_MAGIC  0xaa55
#define LINUX_KERNEL_HDR_MAGIC        0x53726448 // "HdrS"

#define SETUP_HEADER_OFFSET  0x1f1   // look at bootparam.h in linux
#define SETUP_HEADER_FIELD_VAL(boot_params, offset, field_type) \
    (*reinterpret_cast<field_type*>((char*)boot_params + SETUP_HEADER_OFFSET + offset))

#define BOOT_FLAG_OFFSET     sizeof(uint8_t) + 4 * sizeof(uint16_t) + sizeof(uint32_t)
#define HDR_MAGIC_OFFSET     sizeof(uint8_t) + 6 * sizeof(uint16_t) + sizeof(uint32_t)

extern "C" {
[[noreturn]] void _prestart();
[[noreturn]] [[gnu::section(".text.entry")]] void _start(void* boot_params) {
    if(SETUP_HEADER_FIELD_VAL(boot_params, BOOT_FLAG_OFFSET, uint16_t) != LINUX_KERNEL_BOOT_FLAG_MAGIC) {
        while (true);
    }
    if(SETUP_HEADER_FIELD_VAL(boot_params, HDR_MAGIC_OFFSET, uint32_t) != LINUX_KERNEL_HDR_MAGIC) {
        while (true);
    }
    tos::x86_64::set_stack_ptr(reinterpret_cast<char*>(&main_stack) + sizeof main_stack);
    _prestart();
    TOS_UNREACHABLE();
}
}