//
// Created by fatih on 1/5/20.
//

#pragma once

#include <utility>

namespace tos::debug::detail {
template <class LoggerT>
class logger_base {
public:
    template <class... Ts>
    bool operator()(Ts&&... args) {
        self().log(std::forward<Ts>(args)...);
        return true;
    }

private:
    const LoggerT& self() const {
        return *static_cast<const LoggerT*>(this);
    }

    LoggerT& self() {
        return *static_cast<LoggerT*>(this);
    }
};

template <bool Enabled>
struct static_enable {
    [[nodiscard]] bool enabled() const {
        return Enabled;
    }
};

struct dynamic_enable {
protected:
    explicit dynamic_enable(bool enabled = true) : m_enabled(enabled) {}

public:
    [[nodiscard]] bool enabled() const {
        return m_enabled;
    }

    void set_enabled(bool status) {
        m_enabled = status;
    }

private:
    bool m_enabled;
};
}