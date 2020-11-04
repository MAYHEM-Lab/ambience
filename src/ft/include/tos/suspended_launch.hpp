#pragma once

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

namespace tos {
template<class StackT, class FnT, class... Args>
auto& suspended_launch(StackT&& s, FnT&& fn, Args&&... arg) {
    tos::semaphore sem{0};
    auto& res = tos::launch(std::forward<StackT>(s),
                            [&sem,
                             fn = std::forward<FnT>(fn),
                             args = std::tuple<Args...>(std::forward<Args>(arg)...)] {
                                // Let the launcher know we were scheduled and have a
                                // context.
                                sem.up();
                                // Now, suspend the thread.
                                tos::kern::suspend_self(tos::int_guard{});
                                // Someone else made us runnable, call the user entry
                                // point.
                                std::apply(fn, std::move(args));
                            });
    sem.down();
    return res;
}
} // namespace tos