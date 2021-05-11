#pragma once

#include <tos/ae/rings.hpp>
#include <tos/ae/service_host.hpp>

namespace tos::ae {
template<size_t Count>
struct group {
    template<class... Services>
    static group make(Services*... servs) {
        return group(servs...);
    }

    bool run_proc(int channel, int proc, const void* arg, void* res) const {
        auto& serv = m_services[channel];
        return serv.run_zerocopy(proc, arg, res);
    }

private:
    template<class... Services>
    explicit group(Services*... servs)
        : m_services{service_host(servs)...} {
    }

    std::array<service_host, Count> m_services;
};

template<size_t N>
bool run_req(const tos::ae::group<N>& grp, const tos::ae::req_elem& el) {
    return grp.run_proc(el.channel, el.procid, el.arg_ptr, el.ret_ptr);
}
} // namespace tos::ae