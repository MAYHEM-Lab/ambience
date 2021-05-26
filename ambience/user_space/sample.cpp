#include <alarm_generated.hpp>
#include <calc_generated.hpp>
#include <file_system_generated.hpp>
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

auto init_basic_calc(tos::services::logger::async_server* logger,
                     tos::ae::services::alarm::async_server* alarm,
                     tos::ae::services::filesystem::async_server* fs)
    -> tos::Task<tos::ae::services::calculator::async_server*>;
tos::Task<void> task() {
    auto ext_dep1 = transport.get_service<tos::ae::services::filesystem, 3>();
    auto ext_dep2 = transport.get_service<tos::ae::services::alarm, 2>();
    auto ext_dep3 = transport.get_service<tos::services::logger, 1>();

    auto calc = co_await init_basic_calc(ext_dep3, ext_dep2, ext_dep1);

    ::g = new tos::ae::group<1>(tos::ae::group<1>::make(calc));
}
