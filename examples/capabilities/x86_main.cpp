//
// Created by fatih on 3/9/19.
//

#include "apps.hpp"
#include "common.hpp"

#include <arch/x86/drivers.hpp>
#include <fstream>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/streams.hpp>
#include <tos/version.hpp>
#include <vector>

std::vector<char> read_file(const std::string& path) {
    std::ifstream file(path);
    return std::vector<char>(std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>{});
}

static void x86_main() {
    auto tty = tos::open(tos::devs::tty<0>);
    tos::println(tty, "hello");

    //    auto f = read_file("/home/fatih/req.bin");
    //
    //    tos::imemory_stream is{f};

    sink_task(tty);
}

void tos_main() {
    tos::launch(tos::alloc_stack, x86_main);
}