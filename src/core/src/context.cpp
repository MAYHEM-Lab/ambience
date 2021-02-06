#include <array>
#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/memory/malloc.hpp>
#include <tos/utility.hpp>
#include <type_traits>

#if defined(TOS_PLATFORM_raspi)
#include <tos/memory/free_list.hpp>
namespace {
alignas(16) std::array<uint8_t, 16 * 1024 * 1024> heap_mem;
}
#elif defined(TOS_PLATFORM_x86_64)
#include <tos/memory/free_list.hpp>
namespace {
[[gnu::section(".nozero")]]
alignas(16) std::array<uint8_t, 128 * 1024> heap_mem;
}
#elif defined(TOS_PLATFORM_stm32_hal)
extern "C" {
extern uint8_t _estack;
extern uint8_t _end;
}
#include <tos/memory/free_list.hpp>
namespace {
tos::span<uint8_t> heap_memory() {
    return tos::span<uint8_t>{&_end, &_estack};
}
}
#endif

namespace tos {
context& default_context() {
#if defined(TOS_PLATFORM_raspi) || defined(TOS_PLATFORM_x86_64)
    static auto erased_alloc =
        forget(memory::erase_allocator(memory::free_list{heap_mem}));
#elif defined(TOS_PLATFORM_stm32_hal)
    static auto erased_alloc = forget(memory::erase_allocator(memory::free_list{heap_memory()}));
#else
    static auto erased_alloc = forget(memory::erase_allocator(memory::mallocator{}));
#endif
    static auto ctx =
        forget<static_context<allocator_component, debug::logger_component>>(
            erased_alloc.get(), debug::logger_component{});
    return ctx.get();
}
} // namespace tos