#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/memory/malloc.hpp>

namespace tos {
context& default_context() {
    static auto erased_alloc = memory::erase_allocator(memory::mallocator{});
    static static_context<allocator_component> ctx{erased_alloc};
    return ctx;
}
}