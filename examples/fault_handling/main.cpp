#include <tos/arm/assembly.hpp>
#include <tos/arm/exception.hpp>
#include <tos/arm/mpu.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/platform.hpp>
#include <tos/suspended_launch.hpp>

volatile int* ptr = reinterpret_cast<int*>(0xFFFFFFFF);

static constexpr uint16_t instr[] = {0b1101'1110'1111'1111};
int illegal_instruction_execution() {
    auto bad_instruction = (int (*)())instr;
    return bad_instruction();
}

volatile int* nptr;
alignas(32) tos::stack_storage store;
void fault_main() {
    tos::this_thread::yield();
    tos::arm::breakpoint();
}

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(const T& t) {
        m_inst = this;
        NVIC_EnableIRQ(SVCall_IRQn);
        NVIC_SetPriority(SVCall_IRQn, 0);

        m_handler = tos::function_ref<void()>(t);

        tos::arm::svc1();

        NVIC_DisableIRQ(SVCall_IRQn);
    }

    static svc_on_demand_interrupt* instance() {
        return m_inst;
    }

    void run() {
        m_handler();
    }

private:
    inline static svc_on_demand_interrupt* m_inst;
    tos::function_ref<void()> m_handler{[](void*) {}};
};

extern "C" void SVC_Handler() {
    svc_on_demand_interrupt::instance()->run();
}

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

    void* ret_addr;
    const tos::kern::tcb* fault_thread;
    const tos::arm::exception::fault_variant* fault_ptr;
    auto fault_handler = [&](const tos::arm::exception::fault_variant& fault) {
        ret_addr = __builtin_return_address(0);
        fault_thread = tos::self();
        fault_ptr = &fault;
        tos::arm::set_control(0);
        trampoline->switch_to(self_task);
        return true;
    };

    tos::arm::exception::set_general_fault_handler(
        tos::function_ref<bool(const tos::arm::exception::fault_variant&)>(
            fault_handler));

    LOG("Storage:", reinterpret_cast<void*>(&store));
    tos::arm::mpu mpu;
    tos::launch(store, [&] {
        mpu.enable();
        mpu.set_region(0,
                       {reinterpret_cast<uintptr_t>(&store), sizeof(store)},
                       tos::permissions::read_write);
        mpu.set_region(
            1, {0x8000000, 1024 * 1024}, tos::permissions::read_execute, false);

        LOG("MPU setup complete, allowing stack & flash access");

        LOG("Switching to unprivileged mode");
        tos::arm::dmb();
        tos::arm::set_control(1);

        fault_main();
    });

    while (true) {
        using namespace tos::arm::exception;
        tos::kern::suspend_self(tos::int_guard{});
        mpu.disable();
        LOG("Someone died:", const_cast<void*>(static_cast<const void*>(fault_thread)));
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

void tos_main() {
    tos::launch(monitor_stack, monitor_task);
}