#pragma once

#include <lidlrt/concepts.hpp>
#include <lidlrt/zerocopy_vtable.hpp>
#include <tos/concepts.hpp>

namespace tos::ae::kernel {
template<class Interface>
struct fifo_lb_policy {
    Interface* select_next(span<Interface*> backends) {
        auto idx = m_next;
        m_next = (m_next + 1) % backends.size();
        return backends[idx];
    }

    int m_next = 0;
};

template<class Interface, class Policy = fifo_lb_policy<Interface>>
struct load_balancer;

template<lidl::AsyncService Interface, class Policy>
struct load_balancer<Interface, Policy> : Policy {
    explicit load_balancer(span<Interface*> backends)
        : m_backends(backends.begin(), backends.end()) {
    }

    template<IsSame<Interface>... Ts>
    explicit load_balancer(Ts*... backends)
        : m_backends{backends...} {
    }

    auto execute(int proc_id, const void* arg_ptr, void* res_ptr) {
        const auto* vtbl = &lidl::async_vtable<Interface>;
        return vtbl[proc_id](*Policy::select_next(m_backends), arg_ptr, res_ptr);
    }

    std::vector<Interface*> m_backends;
};
} // namespace tos::ae::kernel