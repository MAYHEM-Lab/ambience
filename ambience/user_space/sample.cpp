#include <calc_generated.hpp>
#include <log_generated.hpp>
#include <tos/ae/group.hpp>
#include <tos/ae/transport/upcall.hpp>
#include <tos/task.hpp>

namespace {
constexpr auto queue_len = 32;
[[gnu::section(".nozero")]] tos::ae::interface_storage<queue_len> storage;
} // namespace

tos::ae::interface iface = storage.make_interface();
auto transport = tos::ae::upcall_transport<&iface>{};

tos::ae::group<1>* g;

tos::Task<bool> handle_req(tos::ae::req_elem el) {
    return tos::ae::run_req(*g, el);
}

tos::Task<tos::ae::services::calculator::async_server*>
init_basic_calc(tos::services::logger::async_server* logger);
tos::Task<void> task() {
    auto log_client = transport.get_service<tos::services::logger, 1>();

    ::g = new tos::ae::group<1>(
        tos::ae::group<1>::make(co_await init_basic_calc(log_client)));
}