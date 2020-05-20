//
// Created by Mehmet Fatih BAKIR on 13/04/2018.
//

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <arch/drivers.hpp>
#include <tos/debug/detail/logger_base.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::hosted::stderr_adapter{}),
            tos::hosted::clock<std::chrono::system_clock>{}}));

    tos::launch(tos::alloc_stack, [] {
        doctest::Context context;
        try {
            int res = context.run(); // run

            if (context.shouldExit()) {
                // important - query flags (and --exit) rely on the
                // user doing this
                // propagate the result of the tests
                exit(res);
                return res;
            }
            exit(res);
        } catch (std::exception& error) {
            std::cerr << error.what() << '\n';
            exit(1);
        }
    });
}