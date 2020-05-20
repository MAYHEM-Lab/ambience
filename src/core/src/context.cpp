#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/memory/malloc.hpp>
#include <tos/utility.hpp>
#include <type_traits>

namespace tos {
context& default_context() {
    static auto erased_alloc = forget(memory::erase_allocator(memory::mallocator{}));
    static auto ctx = forget<static_context<allocator_component>>(erased_alloc.get());
    return ctx.get();
}
} // namespace tos