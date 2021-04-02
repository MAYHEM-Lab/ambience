#include <nonstd/variant.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/task.hpp>
#include <tos/x86_64/mmu.hpp>

namespace {
using page_alloc_res = mpark::variant<tos::cur_arch::mmu_errors>;

tos::expected<tos::physical_page_allocator*, page_alloc_res> initialize_page_allocator() {
    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space = tos::align_nearest_up_pow2(
        tos::physical_page_allocator::size_for_pages(1024), 4096);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    EXPECTED_TRYV(
        tos::cur_arch::allocate_region(tos::cur_arch::get_current_translation_table(),
                                       allocator_segment,
                                       tos::user_accessible::no,
                                       nullptr));

    EXPECTED_TRYV(
        tos::cur_arch::mark_resident(tos::cur_arch::get_current_translation_table(),
                                     allocator_segment.range,
                                     tos::memory_types::normal,
                                     vmem_end));

    auto palloc = new (vmem_end) tos::physical_page_allocator(1024);

    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, 4096});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    return palloc;
}

using initialize_res = mpark::variant<page_alloc_res, nullptr_t>;

tos::expected<void, initialize_res> initialize() {
    auto uart = EXPECTED_TRY(tos::x86_64::uart_16550::open());

    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    LOG("Serial and logs enabled");

    auto page_alloc = EXPECTED_TRY(initialize_page_allocator());

    return {};
}

void switch_to_user(void (*ip)()) {
    using namespace tos::x86_64;
    asm volatile("pushf\n"
                 "popq %r11");
    sysret((void*)ip);
}
} // namespace

namespace tos::ae::kernel {
enum class run_result
{
    // User space has nothing else to run.
    yield,
    // User space execution got cancelled early.
    // Used for preemption.
    cancel,
    // User space got a fault.
    fault
};

struct user_space_execution {

    void back_from_fault();
};

void initialize_user_group(user_group& g) {
}
} // namespace tos::ae::kernel

void tos_main() {
    tos::launch(tos::alloc_stack, initialize);
}