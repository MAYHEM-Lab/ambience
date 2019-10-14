//
// Created by fatih on 10/4/19.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/compiler.hpp>
#include <tos/debug/stack_dump.hpp>

template <class LogT>
void NO_INLINE leaf(LogT& log)
{
    tos::debug::dump_stack(log);
}

template <class LogT>
void NO_INLINE middle(LogT& log)
{
    leaf(log);
}

void backtrace_main() {
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600, 8_pin, 6_pin);

    char buf[] = "hello world";
    tos::debug::do_not_optimize(&buf);

    middle(usart);

    tos::this_thread::block_forever();
}

namespace {
tos::stack_storage<2048> stack;
}

void tos_main() {
    tos::launch(stack, backtrace_main);
}
