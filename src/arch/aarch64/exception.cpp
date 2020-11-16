#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/debug/debug.hpp>
#include <tos/debug/log.hpp>

namespace tos::aarch64::exception {
namespace {
svc_handler_t _svc_handler{[](int, stack_frame_t&, void*) {}};
}

svc_handler_t set_svc_handler(svc_handler_t handler) {
    std::swap(_svc_handler, handler);
    return handler;
}

data_abort analyze_data_abort(uint64_t esr, stack_frame_t* frame) {
    auto ISS = esr & 0xFFFFFF;
//    LOG("ISS", itoa(ISS, 2).data());

    auto isv = static_cast<bool>((ISS >> 24) & 0b1);

    if (!isv) {
//        LOG_WARN("Invalid syndrome!");
    }

    auto far = aarch64::get_far_l1();

    auto size = static_cast<access_size>((ISS >> 22) & 0b11);
    auto sign_extend = static_cast<bool>((ISS >> 21) & 1);

    tos::debug::do_not_optimize(far);
    tos::debug::do_not_optimize(size);
    tos::debug::do_not_optimize(sign_extend);

    while (true);
    return data_abort{

    };
}

void sync_handler([[maybe_unused]] uint64_t from, stack_frame_t* frame) {
    auto esr = aarch64::get_esr_l1();

    auto EC = (esr >> 26) & 0b111111;

    auto ISS = esr & 0xFFFFFF;

    switch (static_cast<exception_classes>(EC)) {
    case exception_classes::undefined:
        while (true)
            ;
        break;
    case exception_classes::SVC64: {
        // The LR stores the instruction to return to. So the SVC instruction is the one
        // before it. Thankfully, ARM ISA is sane and we can get the previous instruction
        // rather trivially.
        auto instr = *reinterpret_cast<uint32_t*>(get_elr_l1() - sizeof(uint32_t));
        // Decode the SVC instruction to get the SVC number.
        // https://developer.arm.com/docs/ddi0596/h/base-instructions-alphabetic-order/svc-supervisor-call
        auto svnum = (instr >> 5) & 0xFFFF;
        _svc_handler(svnum, *frame);
        break;
    }
    case exception_classes::instruction_abort:
    case exception_classes::instruction_abort_lower_level:
//        LOG("Instruction abort");
//        LOG("ISS", itoa(ISS, 2).data());
        while (true);
        break;
    case exception_classes::data_abort:
    case exception_classes::data_abort_lower_level:
        analyze_data_abort(esr, frame);
//        LOG("Data abort");
//        LOG("ISS", itoa(ISS, 2).data());
        break;
    default:
//        LOG("Not handled");
        while (true);
        break;
    }
}
} // namespace tos::aarch64::exception

extern "C" {
[[gnu::used]] void sync_handler([[maybe_unused]] uint64_t from,
                                tos::aarch64::exception::stack_frame_t* frame) {
    tos::aarch64::exception::sync_handler(from, frame);
}
}