#include <tos/ae/rings.hpp>

namespace tos::ae {
int interface::host_to_guest_queue_depth() const {
    return host_to_guest->size(size, res_last_seen);
}

int interface::guest_to_host_queue_depth(uint16_t last_seen) const {
    return guest_to_host->size(size, last_seen);
}
}