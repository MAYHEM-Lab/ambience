#include <tos/aarch64/semihosting.hpp>

namespace tos::aarch64 {
uint64_t semihosting::perform_call(uint32_t opcode, uint64_t param) {
    asm("mov w0, %w[opcode]\n"
        "mov x1, %[param]\n"
        :
        : [opcode] "r"(opcode), [param] "r"(param)
        : "w0", "x1");
    hlt_0xf000();
    uint64_t result;
    asm volatile("mov %[result], x0" : [result] "=r"(result) : : "x0");
    return result;
}
} // namespace tos::aarch64