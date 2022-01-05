#pragma once

#include <tos/debug/trace/detail/metric.hpp>
#include <tos/debug/trace/detail/named.hpp>

namespace tos::trace {
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
        register_at();
    }

    void register_at(trace::registry& registry = global::trace_registry()) {
        registry.m_metrics.push_back(*this);
    }

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