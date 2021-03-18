#include <tos/ae/kernel/syscall.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/kernel/rings.hpp>

namespace tos::ae::kernel {
void handle_init(user_group& group, interface& iface) {
    group.iface.user_iface = &iface;
    group.iface.req_last_seen = {};
}

void handle_yield(user_group& group) {
}
}