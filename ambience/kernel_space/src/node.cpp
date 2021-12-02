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

template<class T>
concept HasGroup = requires(T& t) {
    t.group;
};

tos::function_ref<void(tos::ae::kernel::group&)> make_runnable{
    [](tos::ae::kernel::group&, void*) {}};
tos::function_ref<void(tos::span<const uint8_t>)> low_level_writer{
    [](tos::span<const uint8_t>, void*) {}};

void low_level_write(tos::span<const uint8_t> arg) {
    low_level_writer(arg);
}

void maybe_init_xbee(tos::any_alarm& alarm);
tos::result<void> kernel() {
    platform_support support;

    auto serial = support.init_serial();
    auto writer = [&](tos::span<const uint8_t> arg) { serial.write(arg); };
    low_level_writer = tos::function_ref<void(tos::span<const uint8_t>)>(writer);

    tos::println(serial, "ambience node");
    tos::debug::serial_sink sink(&serial);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::info);
    tos::debug::set_default_log(&logger);

    tos::debug::log("Logger initialized");

    support.stage1_init();
    support.stage2_init();

    tos::debug::log("Platform initialized");

    static tos::async_any_alarm_impl async_alarm{&support.get_chrono().alarm};
    registry.register_service<"alarm">(&async_alarm);

    static tos::debug::log_server serv(sink);
    registry.register_service<"logger">(&serv);

#if defined(TOS_AE_HAVE_XBEE)
    maybe_init_xbee(support.get_chrono().alarm);
#endif

    [[maybe_unused]] auto groups = init_all_groups(support.make_args());
    tos::debug::log("Groups initialized");

    tos::semaphore run_sem{0};
    tos::intrusive_list<tos::ae::kernel::group> group_list;
    auto runnable_maker = [&](tos::ae::kernel::group& group) {
        group_list.push_back(group);
        run_sem.up();
    };
    make_runnable = tos::function_ref<void(tos::ae::kernel::group&)>(runnable_maker);

    while (true) {
        run_sem.down();
        auto& g = group_list.front();
        group_list.pop_front();
        g.runner->run(g);
    }

    return {};
}

static tos::stack_storage<TOS_DEFAULT_STACK_SIZE * 2> kern_stack;
void tos_main() {
    set_name(tos::launch(kern_stack, kernel), "Kernel");
}