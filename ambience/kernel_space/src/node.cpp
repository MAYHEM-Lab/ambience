#include "groups.hpp"
#include "registry.hpp"
#include <boost/hana/for_each.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/result.hpp>

registry_t registry;
tos::ae::registry_base& get_registry() {
    return registry;
}

template <class T>
concept HasGroup = requires(T& t) {
    t.group;
};

void maybe_init_xbee(tos::any_alarm& alarm);
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

    maybe_init_xbee(support.get_chrono().alarm);

    auto groups = init_all_groups(support.make_args());
    tos::debug::log("Groups initialized");

    tos::intrusive_list<tos::ae::kernel::group> group_list;
    boost::hana::for_each(groups, [&](auto& g) {
        if constexpr (HasGroup<decltype(g)>) {
            group_list.push_back(*g.group);
        }
    });

    if (group_list.empty()) {
        tos::this_thread::block_forever();
    }
    
    while (true) {
        tos::this_thread::yield();
        auto& g = group_list.front();
        group_list.pop_front();
        g.runner->run(g);
        group_list.push_back(g);
    }

    return {};
}

static tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 2> kern_stack;
void tos_main() {
    set_name(tos::launch(kern_stack, kernel), "Kernel");
}