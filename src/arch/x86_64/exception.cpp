#include <tos/address_space.hpp>
#include <tos/debug/log.hpp>
#include <tos/ft.inl>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/backtrace.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>

using namespace tos::x86_64;

void dump_registers(const exception_frame& frame) {
    tos::debug::error(
        "RIP", (void*)frame.cs, ":", (void*)frame.rip, "\t", "RSP", (void*)frame.ss, ":", (void*)frame.rsp);
    tos::debug::error("RBP", (void*)frame.rbp, "\t", "RAX", (void*)frame.rax);
    tos::debug::error("RBX", (void*)frame.rbx, "\t", "RDI", (void*)frame.rdi);
    tos::debug::error("RSI", (void*)frame.rsi, "\t", "RDX", (void*)frame.rdx);
    tos::debug::error("RCX", (void*)frame.rcx, "\t", "R8", (void*)frame.r8);
}

extern "C" {
void div_by_zero_handler([[maybe_unused]] exception_frame* frame,
                         [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void debug_handler([[maybe_unused]] exception_frame* frame,
                   [[maybe_unused]] uint64_t num) {
    LOG("Debug handler");
//    LOG_ERROR("Call stack:");
//    auto root = std::optional<trace_elem>{{.rbp = frame->rbp, .rip = frame->rip}};
//    while (root) {
//        LOG_ERROR((void*)root->rip);
//        root = backtrace_next(*root);
//    }

    while (true)
        ;
}
void nmi_handler([[maybe_unused]] exception_frame* frame, [[maybe_unused]] uint64_t num) {
    while (true)
        ;
}
void breakpoint_handler([[maybe_unused]] exception_frame* frame,
                        [[maybe_unused]] uint64_t num) {

    tos::debug::log("Breakpoint from:", get_name(*tos::self()));
    dump_registers(*frame);
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
    LOG("Invalid opcode!",
        (int)num,
        (void*)frame,
        (void*)frame->gpr[14],
        (void*)frame->error_code,
        (void*)frame->rip);

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
    LOG("Failing thread:", get_name(*tos::self()));
    LOG_ERROR("GPF!",
              (int)num,
              (void*)frame,
              (void*)frame->error_code,
              (void*)frame->rip,
              "Return CS",
              (void*)frame->cs,
              "Return SS",
              (void*)frame->ss);
    dump_registers(*frame);

//    LOG_ERROR("Call stack:");
//    auto root = std::optional<trace_elem>{{.rbp = frame->rbp, .rip = frame->rip}};
//    while (root) {
//        LOG_ERROR((void*)root->rip);
//        root = backtrace_next(*root);
//    }

    while (true)
        ;
}

void page_fault_handler([[maybe_unused]] exception_frame* frame,
                        [[maybe_unused]] uint64_t num) {
    if ((frame->cs & 0x3) == 0x3) {
        // Fault from user space
    }

    LOG("Failing thread:", tos::self(), get_name(*tos::self()));

    LOG("Page fault!",
        (int)num,
        (void*)frame,
        (void*)frame->error_code,
        (void*)frame->rip,
        "Fault address:",
        (void*)read_cr2());
    dump_registers(*frame);
    if (tos::global::cur_as) {
        if (auto res =
                tos::global::cur_as->m_backend->handle_memory_fault(*frame, read_cr2())) {
            if (force_get(res)) {
                LOG("Handled correctly");
                return;
            }
        }
    }
    LOG("Could not handle");
//    LOG_ERROR("Call stack:");
//    auto root = std::optional<trace_elem>{{.rbp = frame->rbp, .rip = frame->rip}};
//    while (root) {
//        LOG_ERROR((void*)root->rip);
//        root = backtrace_next(*root);
//    }

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