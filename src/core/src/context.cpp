#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/memory/malloc.hpp>
#include <tos/utility.hpp>

namespace tos {
context& default_context() {
    static auto erased_alloc = forget(memory::erase_allocator(memory::mallocator{}));
    static auto ctx =
        forget<static_context<allocator_component, debug::logger_component>>(
            erased_alloc.get(),
            debug::logger_component{});
    return ctx.get();
}
} // namespace tos