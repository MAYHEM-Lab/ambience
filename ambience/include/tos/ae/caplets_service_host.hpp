#pragma once

#include <lidlrt/service.hpp>
#include <tos/caplets.hpp>

namespace tos::ae {
template<class ServBaseT>
struct sync_caplets_service_host {
    template<class ServiceT>
    explicit sync_caplets_service_host(ServiceT* serv)
        : impl{serv}
        , union_runner{
              &lidl::detail::union_caller<typename ServiceT::service_type::sync_server>} {
    }

    bool run(typename ServBaseT::service_type::wire_types::call_union& call,
             lidl::message_builder& response_builder) const {
        return union_runner(*impl, call, response_builder);
    }

    ServBaseT* impl;
    lidl::typed_union_procedure_runner_t<ServBaseT> union_runner;
};

template<caplets::Token TokenT, class... Services>
struct sync_caplets_host {
    explicit sync_caplets_host(Services*... servs)
        : m_services(servs...) {
    }

    using capability_type =
        decltype(std::declval<TokenT&>().frames().back().capabilities().front());

    bool run_message(tos::span<uint8_t> buffer,
                     lidl::message_builder& response_builder) const {
        TokenT& tok = lidl::get_root<TokenT>(buffer);

        for (auto& cap : tok.frames().back().capabilities()) {
            auto res = ((run_if<Services>(cap, response_builder)) || ...);
            if (res) {
                return true;
            }
        }

        return false;
    }

private:
    template<class ServiceT>
    bool run_if(capability_type& cap, lidl::message_builder& response_builder) const {
        constexpr auto index =
            lidl::meta::list_index_of<ServiceT, lidl::meta::list<Services...>>::value;
        if (auto call =
                get_if<typename ServiceT::service_type::wire_types::call_union>(&cap)) {
            return std::get<index>(m_services).run(*call, response_builder);
        }
        return false;
    }

    std::tuple<sync_caplets_service_host<Services>...> m_services;
};
} // namespace tos::ae