#pragma once

#include <tos/debug/trace/metrics/counter.hpp>
#include <tos/debug/trace/metrics/gauge.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/task.hpp>

namespace tos::trace {
struct metric : list_node<metric> {
    struct visitor {
        virtual Task<void> operator()(std::string_view name,
                                      const basic_counter& cnt) = 0;
        virtual Task<void> operator()(std::string_view name, const basic_gauge& cnt) = 0;
        virtual Task<void> operator()(std::string_view name,
                                      const basic_atomic_counter& cnt) = 0;
        virtual ~visitor() = default;
    };

    template<class VisitorT>
    Task<void> visit(VisitorT& vis) {
        struct visitor_impl : visitor {
            Task<void> operator()(std::string_view name,
                                  const basic_counter& cnt) override {
                return (*m_vis)(name, cnt);
            }

            Task<void> operator()(std::string_view name,
                                  const basic_gauge& cnt) override {
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
} // namespace tos::trace