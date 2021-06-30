#include "groups.hpp"
#include "registry.hpp"
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/result.hpp>
#include <boost/hana/for_each.hpp>

registry_t registry;
tos::ae::registry_base& get_registry() {
    return registry;
}

tos::expected<void, tos::common_error> kernel() {
    platform_support support;

    auto serial = support.init_serial();
    tos::println(serial, "ambience node");
    tos::debug::serial_sink sink(&serial);
    tos::debug::detail::any_logger logger(&sink);
    tos::debug::set_default_log(&logger);

    tos::debug::log("Logger initialized");

    support.stage1_init();
    support.stage2_init();

    tos::debug::log("Platform initialized");

    static tos::async_any_alarm_impl async_alarm{&support.get_chrono().alarm};
    registry.register_service<"alarm">(&async_alarm);

    static tos::debug::log_server serv(sink);
    registry.register_service<"logger">(&serv);

    auto groups = init_all_groups(support.make_args());
    tos::debug::log("Groups initialized");

    tos::intrusive_list<tos::ae::kernel::group> group_list;
    boost::hana::for_each(groups, [&](auto& g) {
         group_list.push_back(*g.group);
    });

    tos::launch(tos::alloc_stack, [&] {
        int res = -1;
        tos::semaphore sem{0};
        while (true) {
            using namespace std::chrono_literals;
            tos::this_thread::sleep_for(*support.get_chrono().alarm, 1s);
            tos::debug::log("Calling");
            tos::coro::make_detached([&]() -> tos::Task<void> {
                auto calc = co_await registry.wait<"calc">();
                res = co_await calc->add(3, 4);
                sem.up();
            }());
            sem.down();
            tos::debug::log("3 + 4 =", res);
        }
    });

    while (true) {
        tos::this_thread::yield();
        auto& g = group_list.front();
        group_list.pop_front();
        g.runner->run(g);
        group_list.push_back(g);
    }

    return {};
}

static tos::stack_storage<2048> kern_stack;
void tos_main() {
    set_name(tos::launch(kern_stack, kernel), "Kernel");
}