#include <array>
#include <tos/allocator/free_list.hpp>
#include <tos/allocator/malloc.hpp>
#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/utility.hpp>
#include <type_traits>

#if defined(TOS_PLATFORM_raspi)
namespace {
alignas(16) std::array<uint8_t, 16 * 1024 * 1024> heap_mem;
}
#elif defined(TOS_PLATFORM_x86_64)
namespace {
[[gnu::section(".nozero")]] alignas(16) std::array<uint8_t, 1024 * 1024> heap_mem;
}
#elif defined(TOS_PLATFORM_stm32_hal)
extern "C" {
extern uint8_t _estack;
extern uint8_t _end;
}
namespace {
tos::span<uint8_t> heap_memory() {
    return tos::span<uint8_t>{&_end, &_estack};
}
} // namespace
#elif defined(TOS_PLATFORM_nrf52)
extern "C" {
extern uint8_t __HeapBase;
extern uint8_t __HeapLimit;
}
namespace {
tos::span<uint8_t> heap_memory() {
    return tos::span<uint8_t>{&__HeapBase, &__HeapLimit};
}
} // namespace
#elif defined(TOS_PLATFORM_x86_hosted)
#elif defined(TOS_PLATFORM_ambience_user)
#else
static_assert(false, "No platform specified!");
#endif

namespace tos {
namespace {
auto make_allocator() {
#if defined(TOS_PLATFORM_raspi) || defined(TOS_PLATFORM_x86_64)
    return memory::erase_allocator(memory::free_list{heap_mem});
#elif defined(TOS_PLATFORM_stm32_hal) || defined(TOS_PLATFORM_nrf52)
    return memory::erase_allocator(memory::free_list{heap_memory()});
#else
    return memory::erase_allocator(memory::mallocator{});
#endif
}

memory::polymorphic_allocator& get_allocator() {
    static auto alloc = make_allocator();
    return alloc;
}
} // namespace

context& default_context() {
    static auto ctx =
        forget<static_context<allocator_component, debug::logger_component>>(
            get_allocator(), debug::logger_component{});
    return ctx.get();
}
} // namespace tos