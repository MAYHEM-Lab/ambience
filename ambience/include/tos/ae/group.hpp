#pragma once

#include "tos/debug/log.hpp"
#include "tos/stack_storage.hpp"
#include <tos/ae/rings.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>
#include <tos/fiber/this_fiber.hpp>
#include <tos/debug/trace/metrics/gauge.hpp>

namespace tos::ae {
inline trace::basic_min_max_gauge concurrency;

inline void dispatch_sync_service(const void* serv_ptr, const tos::ae::req_elem& el) {
    auto fn = [serv = static_cast<const sync_service_host*>(serv_ptr),
               ptr = el.user_ptr,
               &el](auto& fib) {
        concurrency.inc();
        serv->run_zerocopy(el.procid, el.arg_ptr, el.ret_ptr);
        done_callback(ptr);
        concurrency.dec();
    };
    // This fiber will release its stack once the request completes.
    auto f = fiber::registered_owning::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, fn);
    if (!f) {
        tos::debug::error("Failed allocation");
    }
    f->resume();
}

inline void dispatch_async_service(const void* serv_ptr, const tos::ae::req_elem& el) {
    auto coro = [](const async_service_host* serv,
                   const tos::ae::req_elem& el,
                   void* ptr) -> coro::detached {
        concurrency.inc();
        co_await serv->run_zerocopy(el.procid, el.arg_ptr, el.ret_ptr);
        done_callback(ptr);
        concurrency.dec();
    };
    coro(static_cast<const async_service_host*>(serv_ptr), el, el.user_ptr);
}

template<size_t Count>
struct group {
    explicit group(lidl::Service auto*... servs)
        : m_services{service_data(servs)...} {
    }

    void run_req(const tos::ae::req_elem& el) const {
        auto& serv = m_services[el.channel];
        serv.fn(&serv, el);
    }

private:
    struct service_data {
        explicit service_data(lidl::AsyncService auto* serv)
            : async{serv}
            , fn{&dispatch_async_service} {
        }
        explicit service_data(lidl::SyncService auto* serv)
            : sync{serv}
            , fn{&dispatch_sync_service} {
        }

        union {
            sync_service_host sync;
            async_service_host async;
        };
        void (*fn)(const void*, const tos::ae::req_elem& el);
    };

    std::array<service_data, Count> m_services;
};

group(lidl::Service auto*... servs) -> group<sizeof...(servs)>;
} // namespace tos::ae