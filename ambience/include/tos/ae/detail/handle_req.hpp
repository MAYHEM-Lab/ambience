#pragma once

#include <tos/ae/service_host.hpp>
#include <tos/detail/poll.hpp>
#include <tos/function_ref.hpp>
#include <tos/semaphore.hpp>

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
} // namespace tos::ae