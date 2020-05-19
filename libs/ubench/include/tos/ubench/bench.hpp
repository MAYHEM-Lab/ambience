//
// Created by fatih on 12/7/19.
//

#pragma once

#include <tos/ubench/state.hpp>
#include <tos/compiler.hpp>

namespace tos::bench {
template<class ClockT>
class bench {
public:
    explicit bench(ClockT clock)
        : m_clock{std::move(clock)} {
    }

    template<class FnT>
    auto do_benchmark(FnT&& fn)
        -> std::tuple<typename std::remove_pointer_t<ClockT>::duration,
                     uint32_t, uint64_t, uint64_t> {
        state<ClockT> s(m_clock);
        fn(s);
        return {s.mean, s.iters, s.sums, s.squares};
    }

private:
    ClockT m_clock;
};

struct benchmark_base;
inline intrusive_list<benchmark_base> benchmarks;

struct benchmark_base : list_node<benchmark_base> {
    explicit benchmark_base(const char* n) : name{n} {
        benchmarks.push_back(*this);
    }
    const char* name;

    virtual void run(any_state& state) = 0;
    virtual ~benchmark_base() = default;
};

template <class T>
struct benchmark : benchmark_base {
    T fn;
    benchmark(const char* name, T f) : benchmark_base{name}, fn{std::move(f)} {}
    void run(any_state& state) override {
        fn(state);
    }
};

template <class T>
benchmark(const char*, T) -> benchmark<T>;

using any_bench = bench<any_clock*>;

template <class T>
void run_global_benchmarks(T& benchmarker) {
    for (auto& bench : benchmarks) {
        benchmarker(bench.name, [&](any_state& state) {
            bench.run(state);
        });
    }
}
} // namespace tos::bench

#define BENCHMARK(name) USED inline ::tos::bench::benchmark name##__{#name, &name}