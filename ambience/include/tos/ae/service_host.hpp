#pragma once

#include <lidlrt/zerocopy_vtable.hpp>

namespace tos::ae {
struct sync_service_host {
    template<class ServiceT>
    explicit sync_service_host(ServiceT* serv)
        : impl{serv}
        , zerocopy_vtable{lidl::make_zerocopy_vtable<typename ServiceT::service_type>()}
        , message_runner{lidl::make_erased_procedure_runner<
              typename ServiceT::service_type::sync_server>()} {
    }

    bool run_zerocopy(int proc, const void* arg, void* res) const {
        return zerocopy_vtable[proc](*impl, arg, res);
    }

    bool run_message(tos::span<uint8_t> buffer,
                     lidl::message_builder& response_builder) const {
        return message_runner(*impl, buffer, response_builder);
    }

    lidl::service_base* impl;
    lidl::zerocopy_vtable_t zerocopy_vtable;
    lidl::erased_procedure_runner_t message_runner;
};

struct async_service_host {
    template<class ServiceT>
    explicit async_service_host(ServiceT* serv)
        : impl{serv}
        , zerocopy_vtable{lidl::make_async_zerocopy_vtable<
              typename ServiceT::service_type>()}
        , message_runner{lidl::make_async_erased_procedure_runner<
              typename ServiceT::service_type::async_server>()} {
    }

    tos::Task<bool> run_zerocopy(int proc, const void* arg, void* res) const {
        return zerocopy_vtable[proc](*impl, arg, res);
    }

    tos::Task<bool> run_message(tos::span<uint8_t> buffer,
                                lidl::message_builder& response_builder) const {
        return message_runner(*impl, buffer, response_builder);
    }

    lidl::service_base* impl;
    lidl::async_zerocopy_vtable_t zerocopy_vtable;
    lidl::async_erased_procedure_runner_t message_runner;
};
} // namespace tos::ae