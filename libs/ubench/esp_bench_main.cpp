#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ubench/bench.hpp>

struct high_resolution_clock {
public:
    using rep = uint32_t;
    using period = std::micro;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<high_resolution_clock>;

    static const bool is_steady = true;

    static time_point now() {
        return time_point{duration(system_get_time())};
    }
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
    auto timer = tos::open(tos::devs::timer<0>);
    tos::alarm alarm(&timer);

    auto usart = open(tos::devs::usart<0>, tos::uart::default_115200);
    tos::debug::serial_sink sink{&usart};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    high_resolution_clock clk;
    auto erased_clock = tos::erase_clock(&clk);

    using namespace tos::tos_literals;
    tos::esp82::gpio g;
    auto power_pin = 5_pin;
    g->set_pin_mode(power_pin, tos::pin_mode::out);
    g->write(power_pin, tos::digital::low);

    tos::bench::bench<tos::any_clock*> bechmark(&erased_clock);
    auto bench_and_print = [&](const auto& name, const auto& fn) {
      tos::debug::log("Running", name);
      g->write(power_pin, tos::digital::high);
      auto [dur, iters, sums, squares] = bechmark.do_benchmark(fn);
      auto mean = sums / iters;
      auto stdev = fisqrt(squares / iters - mean * mean);
      g->write(power_pin, tos::digital::low);
      tos::debug::log(name, ":", dur.count(), iters, int(stdev), sums, squares);
    };

    tos::bench::run_global_benchmarks(bench_and_print);

    LOG("All benchmarks ran, shutting down...");
    tos::this_thread::block_forever();
}

tos::stack_storage<4 * 4096> stack;
void tos_main() {
    tos::launch(stack, bench_main);
}