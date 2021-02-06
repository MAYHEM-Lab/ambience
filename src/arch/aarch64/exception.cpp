#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/aarch64/semihosting.hpp>
#include <tos/debug/debug.hpp>
#include <tos/debug/log.hpp>

extern "C" {
int64_t _irq_count;
int64_t _irq_exit_count;
}

namespace tos::aarch64::exception {
namespace {
svc_handler_t _svc_handler{[](int, stack_frame_t&, void*) {}};
fault_handler_t _fault_handler{[](const fault_variant& fault, stack_frame_t&, void*) {
    tos::aarch64::semihosting::write0("Unhandled fault!\n");
    visit(make_overload(
              [](const data_abort& abort) {
                  tos::aarch64::semihosting::write0("Data abort!\n");
                  tos::aarch64::semihosting::write0(
                      tos::itoa(abort.data_addr, 16).data());
                  tos::aarch64::semihosting::write0("\n");
                  tos::aarch64::semihosting::write0(
                      tos::itoa(abort.return_address, 16).data());
                  tos::aarch64::semihosting::write0("\n");

                  auto& pt = get_current_translation_table();
                  auto entry_res = entry_for_address(pt, abort.data_addr);
                  if (!entry_res) {
                      switch (force_error(entry_res)) {
                      case mmu_errors::not_allocated:
                          tos::aarch64::semihosting::write0(
                              "Address not allocated in virtual memory\n");
                      default:
                          tos::aarch64::semihosting::write0(
                              "Unknown\n");
                          break;
                      }
                      return;
                  }
                  auto entry = force_get(entry_res);
                  if (!entry->valid()) {
                      tos::aarch64::semihosting::write0("Address not resident\n");
                  }
              },
              [](const auto&) { tos::aarch64::semihosting::write0("Unknown fault!\n"); }),
          fault);
    tos::aarch64::semihosting::exit(1);
}};
} // namespace

svc_handler_t set_svc_handler(svc_handler_t handler) {
    std::swap(_svc_handler, handler);
    return handler;
}

fault_handler_t set_fault_handler(fault_handler_t handler) {
    std::swap(_fault_handler, handler);
    return handler;
}

data_abort analyze_data_abort(uint64_t esr, stack_frame_t* frame) {
    auto far = aarch64::get_far_l1();

    const auto ISS = esr & 0x1FFFFFF;
    //    LOG("ISS", itoa(ISS, 2).data());

    auto isv = static_cast<bool>((ISS >> 24) & 0b1);

    if (!isv) {
        return data_abort{
            {},
            far,
            {},
            {},
            ISS,
        };
    }

    auto size = static_cast<access_size>((ISS >> 22) & 0b11);
    auto sign_extend = static_cast<bool>((ISS >> 21) & 1);

    tos::debug::do_not_optimize(far);
    tos::debug::do_not_optimize(size);
    tos::debug::do_not_optimize(sign_extend);

    return data_abort{{}, far, size, sign_extend, ISS};
}

fault_variant analyze_fault(stack_frame_t* frame) {
    const auto esr = aarch64::get_esr_l1();

    auto EC = (esr >> 26) & 0b111111;

    switch (static_cast<exception_classes>(EC)) {
    case exception_classes::undefined:
        return undefined_instruction{};
    case exception_classes::SVC32:
    case exception_classes::SVC64: {
        // The LR stores the instruction to return to. So the SVC instruction is the one
        // before it. Thankfully, ARM ISA is sane and we can get the previous instruction
        // rather trivially.
        auto instr = *reinterpret_cast<uint32_t*>(get_elr_l1() - sizeof(uint32_t));
        // Decode the SVC instruction to get the SVC number.
        // https://developer.arm.com/docs/ddi0596/h/base-instructions-alphabetic-order/svc-supervisor-call
        auto svnum = (instr >> 5) & 0xFFFF;
        _svc_handler(svnum, *frame);
        return svc_exception{{}, svnum};
    }
    case exception_classes::instruction_abort:
    case exception_classes::instruction_abort_lower_level:
        return unknown_fault{};
    case exception_classes::data_abort:
    case exception_classes::data_abort_lower_level:
        return analyze_data_abort(esr, frame);
    }

    return unknown_fault{};
}

void sync_handler([[maybe_unused]] uint64_t from, stack_frame_t* frame) {
    auto fault = analyze_fault(frame);
    if (std::get_if<svc_exception>(&fault)) {
        return;
    }
    std::visit([](auto& f) { f.return_address = get_elr_l1(); }, fault);
    _fault_handler(fault, *frame);
}
} // namespace tos::aarch64::exception

extern "C" {
[[gnu::used]] void sync_handler([[maybe_unused]] uint64_t from,
                                tos::aarch64::exception::stack_frame_t* frame) {
    tos::aarch64::exception::sync_handler(from, frame);
}
}