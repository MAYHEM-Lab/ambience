#pragma once

#include <cstdint>
#include <string_view>
#include <tos/task.hpp>
#include <tos/ae/rings.hpp>
#include <tos/flags.hpp>

namespace tos::ae {
void proc_res_queue(interface& iface);

inline auto&
submit_req(interface& iface, int channel, int proc, const void* params, void* res) {
    auto el_idx = iface.allocate();
    auto& req_el = iface.elems[el_idx].req;
    req_el.flags = elem_flag::req;
    req_el.channel = channel;
    req_el.procid = proc;
    req_el.arg_ptr = params;
    req_el.ret_ptr = res;
    iface.req->elems[iface.req->head_idx++ % iface.size] = el_idx;
    return req_el;
}

tos::Task<void> log_str(std::string_view sv);
} // namespace tos::ae