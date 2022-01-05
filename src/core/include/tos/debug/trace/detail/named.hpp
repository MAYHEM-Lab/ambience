#pragma once

#include <string_view>

namespace tos::trace {
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
} // namespace tos::trace