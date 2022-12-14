#include <arch/drivers.hpp>
#include <tos/arm/core.hpp>
#include <tos/board.hpp>
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

struct cyccnt_clock {
public:
    cyccnt_clock() {
        // Enable CoreDebug_DEMCR_TRCENA_Msk
        tos::arm::SCB::DEMCR.write(tos::arm::SCB::DEMCR.read() | 0x01000000);

        tos::arm::DWT::CYCCNT.write(0);
        tos::arm::DWT::CONTROL.write(tos::arm::DWT::CONTROL.read() | 1);
    }

    typedef std::ratio<1, 64'000'000> clock_cycle;

    using rep = uint64_t;
    using period = clock_cycle;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<cyccnt_clock>;

    [[nodiscard]] time_point now() const {
        return time_point(duration(tos::arm::DWT::CYCCNT.read()));
    }

private:
};

void bench_main() {
    using bs = tos::bsp::board_spec;
    auto base_usart = bs::default_com::open();
    tos::debug::serial_sink sink{&base_usart};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    auto timer = tos::open(tos::devs::timer<1>);
    auto clk = tos::clock(&timer);
    // auto clock = tos::erase_clock(cyccnt_clock());
    auto clock = tos::erase_clock(&clk);

    using namespace tos::tos_literals;
    tos::nrf52::gpio g;
    auto power_pin = 43_pin;
    g->set_pin_mode(power_pin, tos::pin_mode::out);
    g->write(power_pin, tos::digital::low);

    tos::bench::any_bench benchmark(&clock);
    auto bench_and_print = [&](const auto& name, const auto& fn) {
        tos::println(base_usart, "Running", name);
        g->write(power_pin, tos::digital::high);
        auto [dur, iters, sums, squares] = benchmark.do_benchmark(fn);
        auto mean = sums / iters;
        auto stdev = fisqrt(squares / iters - mean * mean);
        g->write(power_pin, tos::digital::low);
        tos::println(
            base_usart, name, ":", int(dur.count()), int(iters), int(stdev), int(sums), int(squares));
    };

    tos::bench::run_global_benchmarks(bench_and_print);
    tos::println(base_usart, "All benchmarks ran, shutting down...");
    tos::this_thread::block_forever();
}

tos::stack_storage<16 * 1024> bench_stack;
void tos_main() {
    tos::launch(bench_stack, bench_main);
}