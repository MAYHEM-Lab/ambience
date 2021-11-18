#pragma once

#include "tos/memory.hpp"
#include <atomic>
#include <cstdint>
#include <string_view>
#include <tos/debug/log.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/task.hpp>

namespace tos::trace {
using count = uint64_t;

struct basic_gauge {
    constexpr void set(uint64_t val) {
        m_count = val;
    }

    constexpr void inc(int by = 1) {
        m_count += by;
    }

    constexpr void dec(int by = 1) {
        m_count -= by;
    }

    uint64_t get() const {
        return m_count;
    }

private:
    uint64_t m_count = 0;
};

struct basic_counter {
    constexpr void inc(int by = 1) {
        m_count += by;
    }

    constexpr void max(uint64_t sample) {
        m_count = std::max<uint64_t>(sample, m_count);
    }

    uint64_t get() const {
        return m_count;
    }

private:
    uint64_t m_count = 0;
};

struct basic_atomic_counter {
    void inc(int by = 1) {
        m_count.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t get() const {
        return m_count.load(std::memory_order_relaxed);
    }

private:
    std::atomic<uint64_t> m_count = 0;
};

template<class BaseMetric>
struct named : BaseMetric {
    template<class... Args>
    constexpr explicit named(std::string_view name, Args&&... args)
        : BaseMetric(std::forward<Args>(args)...)
        , m_name{name} {
    }

    constexpr std::string_view label() const {
        return m_name;
    }

    template<class Visitor>
    decltype(auto) visit(Visitor& vis) {
        return vis(label(), base());
    }

    const BaseMetric& base() const {
        return static_cast<const BaseMetric&>(*this);
    }

private:
    std::string_view m_name;
};

template<class BaseMetric>
named(std::string_view, BaseMetric&&) -> named<BaseMetric>;

struct metric : list_node<metric> {
    struct visitor {
        virtual Task<void> operator()(std::string_view name, const basic_counter& cnt) = 0;
        virtual Task<void> operator()(std::string_view name, const basic_gauge& cnt) = 0;
        virtual Task<void> operator()(std::string_view name,
                                      const basic_atomic_counter& cnt) = 0;
        virtual ~visitor() = default;
    };

    template<class VisitorT>
    Task<void> visit(VisitorT& vis) {
        struct visitor_impl : visitor {
            Task<void> operator()(std::string_view name, const basic_counter& cnt) override {
                return (*m_vis)(name, cnt);
            }

            Task<void> operator()(std::string_view name, const basic_gauge& cnt) override {
                return (*m_vis)(name, cnt);
            }

            Task<void> operator()(std::string_view name,
                                  const basic_atomic_counter& cnt) override {
                return (*m_vis)(name, cnt);
            }

            visitor_impl(VisitorT& vis)
                : m_vis{&vis} {
            }
            VisitorT* m_vis;
        };
        visitor_impl impl(vis);
        return do_visit(impl);
    };

    virtual Task<void> do_visit(visitor& vis) = 0;
    virtual std::string_view label() const = 0;
    virtual ~metric() = default;
};

struct registry {
    intrusive_list<metric> m_metrics;
};
} // namespace tos::trace

namespace tos::global {
inline trace::registry& trace_registry() {
    static trace::registry reg;
    return reg;
}
} // namespace tos::global

namespace tos::trace {
template<class BaseMetric>
struct registerable
    : metric
    , BaseMetric {
    template<class... Args>
    explicit registerable(Args&&... args)
        : BaseMetric(std::forward<Args>(args)...) {
    }

    void register_at(trace::registry& registry = global::trace_registry()) {
        registry.m_metrics.push_back(*this);
    }

    // void dump(dump_target& target) const override {
    //     return target(static_cast<const BaseMetric&>(*this));
    // }

    Task<void> do_visit(visitor& vis) override {
        return BaseMetric::visit(vis);
    }

    std::string_view label() const override {
        if constexpr (requires { BaseMetric::label(); }) {
            return BaseMetric::label();
        } else {
            return "Unknown";
        }
    }
};

template<class BaseMetric>
registerable(BaseMetric&&) -> registerable<BaseMetric>;

using counter = registerable<named<basic_counter>>;
using gauge = registerable<named<basic_gauge>>;
} // namespace tos::trace