#pragma once

#include <tos/ae/service_host.hpp>
#include <tos/detail/poll.hpp>
#include <tos/function_ref.hpp>
#include <tos/semaphore.hpp>
#include <tos/ft.hpp>

namespace tos::ae {
inline void sync_run_message(async_service_host& serv,
                             tos::span<uint8_t> req,
                             lidl::message_builder& response_builder) {
    tos::semaphore exec_sem{0};

    tos::coro::make_detached(serv.run_message(req, response_builder),
                             tos::make_semaphore_upper(exec_sem));

    exec_sem.down();
}

inline void sync_run_message(sync_service_host& serv,
                             tos::span<uint8_t> req,
                             lidl::message_builder& response_builder) {
    serv.run_message(req, response_builder);
}

inline tos::Task<bool> async_run_message(async_service_host& serv,
                                         tos::span<uint8_t> req,
                                         lidl::message_builder& response_builder) {
    return serv.run_message(req, response_builder);
}

inline tos::Task<bool> async_run_message(sync_service_host& serv,
                                         tos::span<uint8_t> req,
                                         lidl::message_builder& response_builder) {
    tos::semaphore sem{0};
    tos::launch(tos::alloc_stack, [req, &response_builder, &serv, &sem]() mutable {
        serv.run_message(req, response_builder);
        sem.up();
    });
    co_await sem;
    co_return true;
}
} // namespace tos::ae