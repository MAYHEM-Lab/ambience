#include "include/tos/fiber.hpp"
#include "tos/fiber/start.hpp"
#include "tos/stack_storage.hpp"
#include <tos/fiber.hpp>

namespace tos::fiber {
namespace {
tos::stack_storage<1024> stak;
void fiber_test() {
    auto f = fiber::non_owning::start(stak, [](basic_fiber& self){
        self.suspend();
    });
    f->resume();
}
}
}