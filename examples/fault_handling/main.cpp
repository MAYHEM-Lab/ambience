#include <this_thread_generated.hpp>
#include <tos/arm/exception.hpp>
#include <tos/arm/mpu.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/platform.hpp>


alignas(1024) tos::stack_storage store;
void fault_main();

class svc_on_demand_interrupt {
public:
    svc_on_demand_interrupt() {
        NVIC_EnableIRQ(SVCall_IRQn);
        NVIC_SetPriority(SVCall_IRQn, 0);
    }

    template<class T>
    void operator()(T&& t) {
        tos::arm::exception::set_svc_handler(tos::arm::exception::svc_handler_t(t));
        tos::arm::svc1();
    }
};

class thread_man_impl final : public tos::services::threadman {
    virtual bool schedule(const uint32_t& thread_id) override {
        auto tcb = reinterpret_cast<tos::kern::tcb*>(thread_id);
        return true;
    }

    virtual bool kill(const uint32_t& thread_id) override {
        auto tcb = reinterpret_cast<tos::kern::tcb*>(thread_id);
        return true;
    }
};

tos::stack_storage monitor_stack;
void monitor_task() {
    auto& self_task = *tos::self();
    auto com = tos::bsp::board_spec::default_com::open();
    tos::debug::serial_sink sink(&com);
    tos::debug::detail::any_logger logger(&sink);
    tos::debug::set_default_log(&logger);

    LOG("Hello!");

    svc_on_demand_interrupt odi{};
    auto trampoline = tos::make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    bool in_interrupt = false;

    void* ret_addr;
    const tos::kern::tcb* fault_thread;
    const tos::arm::exception::fault_variant* fault_ptr;
    auto fault_handler = [&](const tos::arm::exception::fault_variant& fault) {
        tos::arm::set_control(0);
        tos::arm::isb();

        ret_addr = __builtin_return_address(0);
        fault_thread = tos::self();
        fault_ptr = &fault;
        if (in_interrupt) {
            tos::swap_context(*tos::self(), self_task, tos::int_ctx{});
            return true;
        }
        trampoline->switch_to(self_task);
        return true;
    };

    tos::arm::exception::set_general_fault_handler(
        tos::function_ref<bool(const tos::arm::exception::fault_variant&)>(
            fault_handler));

    int svc_num;
    tos::arm::exception::stack_frame_t* svc_frame;
    auto task_svc_handler = [&](int svc, tos::arm::exception::stack_frame_t& frame) {
        if (svc == 0x66) {
            frame.r0 = reinterpret_cast<uint32_t>(tos::self());
            return;
        }

        tos::arm::set_control(0);
        tos::arm::isb();

        svc_num = svc;
        svc_frame = &frame;
        if (in_interrupt) {
            tos::swap_context(*tos::self(), self_task, tos::int_ctx{});
            return;
        }
        trampoline->switch_to(self_task);
        asm volatile("nop");
    };

    LOG("Storage:", reinterpret_cast<void*>(&store));
    tos::arm::mpu mpu;
    auto pre_sched = [&] {
        mpu.enable();
        mpu.set_region(0,
                       {reinterpret_cast<uintptr_t>(&store), sizeof(store)},
                       tos::permissions::read_write);
        mpu.set_region(
            1, {0x800'00'00, 1024 * 1024}, tos::permissions::read_execute, false);
        tos::arm::exception::set_svc_handler(
            tos::arm::exception::svc_handler_t(task_svc_handler));

        //        LOG("MPU setup complete, allowing stack & flash access");

        //        LOG("Switching to unprivileged mode");
        tos::arm::set_control(1);
        tos::arm::isb();
    };

    auto& t = tos::launch(store, [&] {
        pre_sched();
        fault_main();
    });

    tos::intrusive_list<tos::job> runnable;

    while (true) {
        using namespace tos::arm::exception;
        fault_ptr = nullptr;
        svc_frame = nullptr;

        if (!runnable.empty()) {
            auto& front = static_cast<tos::kern::tcb&>(runnable.front());
            runnable.pop_front();
            in_interrupt = true;
            odi([&](auto&&...) {
                pre_sched();
                tos::swap_context(*tos::self(), front, tos::int_ctx{});
            });
            in_interrupt = false;
        } else {
            tos::kern::suspend_self(tos::int_guard{});
        }

        if (svc_frame) {
            LOG("Received syscall", svc_num);
            LOG("Channel:", svc_frame->r0);
            LOG("Data:", (void*)svc_frame->r1);
            LOG("Size:", svc_frame->r2);
            svc_frame->r0 = 1234;
            runnable.push_back(t);
        }

        if (fault_ptr) {
            LOG("Someone died:",
                const_cast<void*>(static_cast<const void*>(fault_thread)));
            LOG("Control:", tos::arm::get_control());
            LOG("From:", ret_addr);
            std::visit(tos::make_overload(
                           [](const undefined_instruction_fault& fault) {
                               LOG("Undefined instruction fault at",
                                   reinterpret_cast<void*>(fault.instr_address));
                           },
                           [](const bus_fault_t& fault) {
                               LOG("Bus error at",
                                   reinterpret_cast<void*>(fault.instr_address),
                                   "On",
                                   reinterpret_cast<void*>(fault.fault_address));
                           },
                           [](const memory_fault& fault) {
                               LOG("Memory fault at",
                                   reinterpret_cast<void*>(fault.instr_address),
                                   "on",
                                   reinterpret_cast<void*>(fault.data_address));
                           },
                           [](const auto& fault) {
                               LOG("Unknown fault at",
                                   reinterpret_cast<void*>(fault.instr_address));
                           }),
                       *fault_ptr);
        }
    }
}

void tos_main() {
    tos::launch(monitor_stack, monitor_task);
}