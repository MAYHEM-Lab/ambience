#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/memory/malloc.hpp>

#if defined(TOS_PLATFORM_raspi)
#include <tos/memory/free_list.hpp>
namespace {
alignas(16) std::array<uint8_t, 16 * 1024 * 1024> heap_mem;
}
#endif

namespace tos {
context& default_context() {
#if defined(TOS_PLATFORM_raspi)
    static auto erased_alloc = memory::erase_allocator(memory::free_list{heap_mem});
#else
    static auto erased_alloc = memory::erase_allocator(memory::mallocator{});
#endif
    static static_context<allocator_component> ctx{erased_alloc};
    return ctx;
}
} // namespace tos