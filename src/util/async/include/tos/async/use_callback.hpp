#pragma once

#include <tos/function_ref.hpp>

namespace tos::async {
template<class... ArgTs>
struct use_callback {
    explicit use_callback(function_ref<void(ArgTs...)> cb)
        : m_cb{cb} {
    }
    template<class Start>
    auto get_completion(Start&& start, function_ref<void(ArgTs...)>& cb) {
        cb = m_cb;
        start();
    }
    function_ref<void(ArgTs...)> m_cb;
};
} // namespace tos::async