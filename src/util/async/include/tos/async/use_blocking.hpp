#pragma once

#include <tos/function_ref.hpp>
#include <tos/semaphore.hpp>

namespace tos::async {
struct use_blocking {
    template<class Start, class... ArgTs>
    auto get_completion(Start&& start, function_ref<void(ArgTs...)>& cb) {
        tos::semaphore sem{0};
        std::tuple<ArgTs...> m_res;
        auto handler = [&](auto&&... vals) {
            sem.up_isr();
            m_res = std::tuple(std::forward<decltype(vals)>(vals)...);
        };
        cb = function_ref<void(ArgTs...)>(handler);
        start();
        sem.down();
        return m_res;
    }
};
} // namespace tos::async