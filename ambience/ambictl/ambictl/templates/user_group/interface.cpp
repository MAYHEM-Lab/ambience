#include <tos/ae/group.hpp>

namespace {
constexpr auto queue_len = {{queue_size}};
[[gnu::section(".nozero")]] tos::ae::interface_storage<queue_len> storage;
} // namespace

tos::ae::interface iface = storage.make_interface();