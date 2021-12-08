#include "common/clock.hpp"
#include "tos/x86_64/assembly.hpp"
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/span.hpp>
#include <tos/ubench/bench.hpp>

void low_level_write(tos::span<const uint8_t>) {
}

struct tsc_clock {
public:
    tsc_clock() {
    }

    typedef std::ratio<1, 1'000'000> clock_cycle;

    using rep = uint64_t;
    using period = clock_cycle;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<tsc_clock>;

    [[nodiscard]] time_point now() const {
        return time_point(duration(tos::x86_64::rdtsc()));
    }

private:
};


int ftbl[33] = {0,    1,    1,    2,    2,    4,    5,     8,     11,    16,    22,
                32,   45,   64,   90,   128,  181,  256,   362,   512,   724,   1024,
                1448, 2048, 2896, 4096, 5792, 8192, 11585, 16384, 23170, 32768, 46340};
int ftbl2[32] = {32768, 33276, 33776, 34269, 34755, 35235, 35708, 36174,
                 36635, 37090, 37540, 37984, 38423, 38858, 39287, 39712,
                 40132, 40548, 40960, 41367, 41771, 42170, 42566, 42959,
                 43347, 43733, 44115, 44493, 44869, 45241, 45611, 45977};

int fisqrt(int val) {
    int cnt = 0;
    int t = val;
    while (t) {
        cnt++;
        t >>= 1;
    }
    if (6 >= cnt)
        t = (val << (6 - cnt));
    else
        t = (val >> (cnt - 6));

    return (ftbl[cnt] * ftbl2[t & 31]) >> 15;
}

void bench_main() {
    using bs = tos::bsp::board_spec;
    auto base_usart = bs::default_com::open();
    tos::debug::serial_sink sink{&base_usart};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    auto clock = tos::erase_clock(tsc_clock{});

    auto vmem_end = tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space =
        tos::align_nearest_up_pow2(tos::physical_page_allocator::size_for_pages(1024),
                                   tos::cur_arch::page_size_bytes);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::physical_segment{tos::physical_range{tos::default_segments::image().end(),
                                                  ptrdiff_t(allocator_space)},
                              tos::permissions::read_write};

    auto op_res =
        tos::cur_arch::map_region(tos::cur_arch::get_current_translation_table(),
                                  identity_map(allocator_segment),
                                  tos::user_accessible::no,
                                  tos::memory_types::normal,
                                  nullptr,
                                  vmem_end);
    LOG(bool(op_res));

    using namespace tos::address_literals;
    auto palloc = new (vmem_end.direct_mapped()) tos::physical_page_allocator(1024);
    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0_physical, tos::cur_arch::page_size_bytes});
    palloc->mark_unavailable({0x00080000_physical, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    tos::bench::any_bench benchmark(&clock);
    auto bench_and_print = [&](const auto& name, const auto& fn) {
        tos::println(base_usart, "Running", name);
        auto [dur, iters, sums, squares] = benchmark.do_benchmark(fn);
        auto mean = sums / iters;
        auto stdev = fisqrt(squares / iters - mean * mean);
        tos::println(base_usart,
                     name,
                     ":",
                     int(dur.count()),
                     int(iters),
                     int(stdev),
                     int(sums),
                     int(squares));
    };

    tos::bench::run_global_benchmarks(bench_and_print);
    tos::println(base_usart, "All benchmarks ran, shutting down...");
    tos::x86_64::port(0x604).outw(0x2000);
    tos::this_thread::block_forever();
}

tos::stack_storage<16 * 1024> bench_stack;
void tos_main() {
    tos::launch(bench_stack, bench_main);
}