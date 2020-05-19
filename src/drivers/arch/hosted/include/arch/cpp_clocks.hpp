#pragma once

#include <common/clock.hpp>

namespace tos::hosted {
template<class ClockT>
class clock : public any_clock {
public:
    time_point now() const override {
        auto tm = ClockT::now().time_since_epoch();
        return time_point{std::chrono::duration_cast<std::chrono::microseconds>(tm)};
    }
};
} // namespace tos::hosted