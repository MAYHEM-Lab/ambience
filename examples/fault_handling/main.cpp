#include <tos/arm/assembly.hpp>
#include <tos/arm/exception.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/platform.hpp>
#include <tos/suspended_launch.hpp>

volatile int* ptr = reinterpret_cast<int*>(0xFFFFFFFF);

static constexpr uint16_t instr[] = {0b1101'1110'1111'1111};
int illegal_instruction_execution() {
    auto bad_instruction = (int (*)())instr;
    return bad_instruction();
}

tos::stack_storage store;
void fault_main() {
    LOG("About to execute an illegal instruction at",
        const_cast<void*>(static_cast<const void*>(&instr)));
    illegal_instruction_execution();
    *ptr = 42;
    tos::this_thread::block_forever();
}

/**
 * This class is used to setup a trampoline that can be used to transition
 * from a thread in interrupt context to another thread in normal mode.
 */
class interrupt_trampoline : public tos::non_copy_movable {
    enum class state
    {
        none,
        first,
        later
    } m_isr_state{};

public:
    /**
     * This function is used as the entry point for the trampoline.
     *
     * Upon entry, it immediately transitions to an interrupt context
     * using a platform dependent function. For instance, it could be an SVC
     * or waiting for a timer interrupt.
     */
    template<class InISR>
    void operator()(InISR& in_isr) {
        in_isr(tos::mem_function_ref<&interrupt_trampoline::on_svc>(*this));
        // The stack up until this point allows us to escape from an interrupt handler.
        // We'll keep that in m_stack and switch to m_tmp_stack to perform the
        // context switch and block_forever.
        tos::cur_arch::set_stack_ptr(reinterpret_cast<char*>(&m_tmp_stack));
        if (m_isr_state == state::first) {
            // if we just saved the context, no need to return anywhere, we'll stay here
            tos::this_thread::block_forever();
        }
        // At this point, we successfully escaped from an interrupt context to thread
        // context, we can safely switch to the requested thread.
        // The thread context that was interrupted will safely stay in a semi-interrupt
        // context.
        tos::kern::switch_context(m_target->get_processor_state(),
                                  tos::return_codes::scheduled);
    }

    template<class InISR>
    void setup(InISR& in_isr) {
        auto& t = tos::suspended_launch(m_stack, std::ref(*this), in_isr);
        LOG("About to swap to trampoline setup");
        tos::kern::make_runnable(*tos::self());
        tos::swap_context(*tos::self(), t);
    }

    void switch_to(tos::kern::tcb& target) {
        m_target = &target;
        tos::swap_context(*tos::self(), m_isr_tcb);
    }

private:
    void on_svc() {
        if (save_context(m_isr_tcb, m_isr_ctx) == tos::return_codes::saved) {
            m_isr_state = state::first;
            return;
        }
        m_isr_state = state::later;
    }

    /**
     * Objects of this type is used as a thread control block for the
     * ISR context that we record. As the thread is started using the
     * regular tos::launch call, there's no need to implement any startup
     * code in this tcb.
     */
    class trampoline_tcb : public tos::kern::tcb {
    public:
        trampoline_tcb(tos::context& ctx)
            : tcb(ctx) {
        }
    };
    trampoline_tcb m_isr_tcb{tos::current_context()};

    /**
     * This context stores an execution context that's in an interrupt
     * handler.
     */
    tos::kern::processor_state m_isr_ctx;

    // Upon executing the trampoline, we'll return to this thread.
    tos::kern::tcb* m_target;
    // This stack maintains the interrupt return code
    tos::stack_storage<378> m_stack;
    // This stack is used to execute block_forever and switch_context calls.
    tos::stack_storage<128> m_tmp_stack;
};

template<class InISR>
auto make_interrupt_trampoline(InISR& isr) {
    auto trampoline = std::make_unique<interrupt_trampoline>();
    trampoline->setup(isr);
    return trampoline;
}

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(const T& t) {
        m_inst = this;
        HAL_NVIC_EnableIRQ(SVCall_IRQn);
        HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);

        m_handler = tos::function_ref<void()>(t);

        tos::arm::svc127();

        HAL_NVIC_DisableIRQ(SVCall_IRQn);
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
    auto trampoline = make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    const tos::kern::tcb* fault_thread;
    const tos::arm::exception::fault_variant* fault_ptr;
    auto fault_handler = [&](const tos::arm::exception::fault_variant& fault) {
        fault_thread = tos::self();
        fault_ptr = &fault;
        trampoline->switch_to(self_task);
        return true;
    };

    tos::arm::exception::set_general_fault_handler(
        tos::function_ref<bool(const tos::arm::exception::fault_variant&)>(
            fault_handler));

    tos::launch(store, fault_main);

    while (true) {
        using namespace tos::arm::exception;
        tos::kern::suspend_self(tos::int_guard{});
        LOG("Someone died:", const_cast<void*>(static_cast<const void*>(fault_thread)));
        std::visit(tos::make_overload(
                       [](const undefined_instruction_fault& fault) {
                           LOG("Undefined instruction fault at",
                               reinterpret_cast<void*>(fault.instr_address));
                       },
                       [](const auto&) { LOG("Unknown fault!"); }),
                   *fault_ptr);
    }
}

void tos_main() {
    tos::launch(monitor_stack, monitor_task);
}