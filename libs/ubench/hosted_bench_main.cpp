#include <arch/drivers.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/ubench/bench.hpp>

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
    auto clock =
        tos::erase_clock(tos::hosted::clock<std::chrono::high_resolution_clock>());

    tos::bench::any_bench benchmark(&clock);
    auto bench_and_print = [&](const auto& name, const auto& fn) {
        tos::debug::log(name);
        // g->write(power_pin, tos::digital::high);
        auto [dur, iters, sums, squares] = benchmark.do_benchmark(fn);
        auto mean = sums / iters;
        auto stdev = fisqrt(squares / iters - mean * mean);
        // g->write(power_pin, tos::digital::low);
        tos::debug::log(name, ":", dur.count(), iters, int(stdev), sums, squares);
    };

    tos::bench::run_global_benchmarks(bench_and_print);
    LOG("All benchmarks ran, shutting down...");
}

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::hosted::stderr_adapter{}),
            tos::hosted::clock<std::chrono::system_clock>{}}));

    tos::launch(tos::alloc_stack, bench_main);
}