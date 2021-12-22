#pragma once

#include <lidlrt/zerocopy_vtable.hpp>
#include <nonstd/variant.hpp>

namespace tos::ae {
struct async_dynamic_host {
    lidl::async_zerocopy_vtable_t zerocopy_vtable;
    lidl::async_erased_procedure_runner_t message_runner;

    template<lidl::AsyncService ServiceT>
    static constexpr async_dynamic_host host_for() {
        return async_dynamic_host{
            .zerocopy_vtable =
                lidl::make_async_zerocopy_vtable<typename ServiceT::service_type>(),
            .message_runner = lidl::make_async_erased_procedure_runner<
                typename ServiceT::service_type::async_server>()};
    }
};
struct sync_dynamic_host {
    lidl::zerocopy_vtable_t zerocopy_vtable;
    lidl::erased_procedure_runner_t message_runner;

    template<lidl::SyncService ServiceT>
    static constexpr sync_dynamic_host host_for() {
        return sync_dynamic_host{
            .zerocopy_vtable =
                lidl::make_zerocopy_vtable<typename ServiceT::service_type>(),
            .message_runner = lidl::make_erased_procedure_runner<
                typename ServiceT::service_type::sync_server>()};
    }
};

struct sync_service_host : sync_dynamic_host {
    template<class ServiceT>
    explicit sync_service_host(ServiceT* serv)
        : sync_dynamic_host(sync_dynamic_host::host_for<ServiceT>())
        , impl{serv} {
    }

    bool run_zerocopy(int proc, const void* arg, void* res) const {
        return zerocopy_vtable[proc](*impl, arg, res);
    }

    bool run_message(tos::span<uint8_t> buffer,
                     lidl::message_builder& response_builder) const {
        return message_runner(*impl, buffer, response_builder);
    }

    lidl::sync_service_base* impl;
};

struct async_service_host : async_dynamic_host {
    template<class ServiceT>
    explicit async_service_host(ServiceT* serv)
        : async_dynamic_host(async_dynamic_host::host_for<ServiceT>())
        , impl{serv} {
    }

    tos::Task<bool> run_zerocopy(int proc, const void* arg, void* res) const {
        return zerocopy_vtable[proc](*impl, arg, res);
    }

    tos::Task<bool> run_message(tos::span<uint8_t> buffer,
                                lidl::message_builder& response_builder) const {
        return message_runner(*impl, buffer, response_builder);
    }

    lidl::async_service_base* impl;
};

template<
    class ServiceT,
    std::enable_if_t<std::is_base_of_v<lidl::async_service_base, ServiceT>>* = nullptr>
async_service_host service_host(ServiceT* ptr) {
    return async_service_host(ptr);
}

template<
    class ServiceT,
    std::enable_if_t<std::is_base_of_v<lidl::sync_service_base, ServiceT>>* = nullptr>
sync_service_host service_host(ServiceT* ptr) {
    return sync_service_host(ptr);
}

using any_service_host = mpark::variant<sync_service_host, async_service_host>;
} // namespace tos::ae