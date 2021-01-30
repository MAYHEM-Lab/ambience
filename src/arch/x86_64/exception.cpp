#include <tos/debug/log.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::x86_64 {}

using namespace tos::x86_64;

extern "C" {
void div_by_zero_handler([[maybe_unused]] exception_frame* frame,
                         [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void debug_handler([[maybe_unused]] exception_frame* frame,
                   [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void nmi_handler([[maybe_unused]] exception_frame* frame, [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void breakpoint_handler([[maybe_unused]] exception_frame* frame,
                        [[maybe_unused]] uint64_t num) {
    LOG("Breakpoint!",
        (int)num,
        (void*)frame,
        (void*)frame->gpr[14],
        (void*)frame->error_code,
        (void*)frame->rip);
}
void overflow_handler([[maybe_unused]] exception_frame* frame,
                      [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void out_of_bounds_handler([[maybe_unused]] exception_frame* frame,
                           [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void invalid_opcode_handler([[maybe_unused]] exception_frame* frame,
                            [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void device_not_available_handler([[maybe_unused]] exception_frame* frame,
                                  [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void double_fault_handler([[maybe_unused]] exception_frame* frame,
                          [[maybe_unused]] uint64_t num) {
    LOG("Double fault!");
    while (true)
        ;
}
void coprocessor_seg_overrun_handler([[maybe_unused]] exception_frame* frame,
                                     [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void invalid_tss_handler([[maybe_unused]] exception_frame* frame,
                         [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void segment_not_present_handler([[maybe_unused]] exception_frame* frame,
                                 [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void stack_segment_fault_handler([[maybe_unused]] exception_frame* frame,
                                 [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void general_protection_fault_handler([[maybe_unused]] exception_frame* frame,
                                      [[maybe_unused]] uint64_t num) {
    LOG("GPF!", (int)num, (void*)frame->error_code, (void*)frame->rip);
    while (true)
        ;
}
void page_fault_handler([[maybe_unused]] exception_frame* frame,
                        [[maybe_unused]] uint64_t num) {
    LOG("Page fault!",
        (int)num,
        (void*)frame,
        (void*)frame->error_code,
        (void*)frame->rip,
        (void*)read_cr2());
    LOG("Handled correctly, hanging");
    while (true)
        ;
}
void x87_fpu_fault_handler([[maybe_unused]] exception_frame* frame,
                           [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void alignment_check_handler([[maybe_unused]] exception_frame* frame,
                             [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void machine_check_handler([[maybe_unused]] exception_frame* frame,
                           [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void simd_fpu_fault_handler([[maybe_unused]] exception_frame* frame,
                            [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void virt_handler([[maybe_unused]] exception_frame* frame,
                  [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void security_exception_handler([[maybe_unused]] exception_frame* frame,
                                [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
}