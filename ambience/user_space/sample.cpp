#include <calc_generated.hpp>
#include <log_generated.hpp>
#include <tos/ae/group.hpp>
#include <tos/ae/transport/upcall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/task.hpp>

extern tos::ae::interface iface;

tos::ae::group<1>* g;

tos::Task<bool> handle_req(tos::ae::req_elem el) {
    return tos::ae::run_req(*g, el);
    /*std::array<uint8_t, 128> buf;
    tos::omemory_stream ostr(buf);
    tos::print(ostr,
               "Received request",
               int(el.channel),
               int(el.procid),
               el.arg_ptr,
               el.ret_ptr,
               el.user_ptr);

    co_await tos::ae::log_str(
        std::string_view((const char*)ostr.get().data(), ostr.get().size()));

    auto res = co_await tos::ae::run_req(*g, el);

    co_await tos::ae::log_str("Handled");

    co_return res;*/
}

tos::Task<tos::ae::services::calculator::async_server*> init_basic_calc();
tos::Task<void> task() {
    tos::services::logger::async_zerocopy_client<tos::ae::upcall_transport> client(iface,
                                                                                   1);

    auto calc = co_await init_basic_calc();
    ::g = new tos::ae::group<1>(tos::ae::group<1>::make(calc));

    co_await tos::ae::log_str("Group fully initialized");

    co_await calc->add(3, 5);

    co_await client.start(tos::services::log_level::info);
    co_await client.log_string("Hello world!");
    co_await client.finish();
}