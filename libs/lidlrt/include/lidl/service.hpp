//
// Created by fatih on 1/24/20.
//

#pragma once

#include <lidl/builder.hpp>
#include <lidl/meta.hpp>
#include <lidl/status.hpp>
#include <string_view>
#include <tuple>

namespace lidl {
template<class T>
class service_descriptor;

template<auto Fn, class ParamsT, class ResultsT>
struct procedure_descriptor {
    std::string_view name;
};

template<class...>
struct procedure_traits;

template<class Type, class RetType, class... ArgTypes>
struct procedure_traits<RetType (Type::*const)(ArgTypes...)> {
    using return_type = RetType;
    using param_types = meta::list<ArgTypes...>;

    static constexpr bool takes_response_builder() {
        return (... ||
                std::is_same_v<std::remove_reference_t<ArgTypes>, message_builder>);
    }
};

template<class Type, class RetType, class... ArgTypes>
struct procedure_traits<RetType (Type::*)(ArgTypes...)>
    : procedure_traits<RetType (Type::*const)(ArgTypes...)> {};

template<class ServiceT>
class service_call_union;

template<class ServiceT>
class service_return_union;

template<class T>
class rpc_param_traits;

class service_base {
public:
    virtual ~service_base()               = default;
    virtual std::string_view name() const = 0;
};

template<class ServiceT>
class service : public service_base {
public:
    std::string_view name() const override {
        return service_descriptor<ServiceT>::name;
    }

    using call_union   = service_call_union<ServiceT>;
    using return_union = service_return_union<ServiceT>;
};

template<class T>
tos::span<uint8_t> data(T&);

namespace meta {
template<class... ParamsT>
struct get_result_type_impl;

template<auto... Procs, class... ParamsT, class... ResultsT>
struct get_result_type_impl<
    const std::tuple<lidl::procedure_descriptor<Procs, ParamsT, ResultsT>...>> {
    using params  = std::tuple<ParamsT...>;
    using results = std::tuple<ResultsT...>;
};
} // namespace meta

using procedure_runner_t = void (*)(service_base&,
                                    tos::span<const uint8_t>,
                                    lidl::message_builder&);

namespace detail {
template<class ServiceT>
void request_handler(service_base& base_service,
                     tos::span<const uint8_t> buffer,
                     lidl::message_builder& response) {
    auto& service = static_cast<ServiceT&>(base_service);

    /**
     * Don't panic!
     *
     * Yes, this is full of fearsome metaprogramming magic, but I'll walk you through.
     */

    // The service descriptor stores the list of procedures in a service. We'll use
    // this information to decode lidl messages into actual calls to services.
    using descriptor = service_descriptor<ServiceT>;

    using params_union  = typename ServiceT::call_union;
    using results_union = typename ServiceT::return_union;

    using all_params =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::params;
    using all_results =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::results;

    visit(
        [&](const auto& call_params) -> decltype(auto) {
            constexpr auto proc = rpc_param_traits<std::remove_const_t<
                std::remove_reference_t<decltype(call_params)>>>::params_for;
            using proc_traits   = procedure_traits<decltype(proc)>;
            constexpr auto idx  = meta::tuple_index_of<
                std::remove_const_t<std::remove_reference_t<decltype(call_params)>>,
                all_params>::value;
            using result_type = std::remove_const_t<std::remove_reference_t<decltype(
                std::get<idx>(std::declval<all_results>()))>>;

            /**
             * This ugly thing is where the final magic happens.
             *
             * The apply call will pass each member of the parameters of the call to
             * this function.
             *
             * Inside, we have a bunch of cases:
             *
             * 1. Does the procedure take a message builder or not?
             *
             *    Procedures that do not return a _reference type_ (types that contain
             *    pointers) do not need a message builder since their result will be
             *    self contained.
             *
             * 2. Is the return value a view type?
             *
             *    Procedures that return a view type need special care. The special
             *    care is basically that we copy whatever it returns to the response
             *    buffer.
             *
             *    If not, we return whatever the procedure returned directly.
             *
             */
            auto make_service_call = [&service, &response](const auto&... args) -> void {
                constexpr auto proc = rpc_param_traits<std::remove_const_t<
                    std::remove_reference_t<decltype(call_params)>>>::params_for;

                if constexpr (!proc_traits::takes_response_builder()) {
                    auto res = (service.*(proc))(args...);
                    create<results_union>(response, result_type(res));
                } else {
                    const auto& res = (service.*(proc))(args..., response);
                    if constexpr (std::is_same_v<meta::remove_cref<decltype(res)>,
                                                 std::string_view>) {
                        /**
                         * The procedure returned a view.
                         *
                         * We need to see if the returned view is already in the
                         * response buffer. If it is not, we will copy it.
                         *
                         * Issue #6.
                         */

                        auto& str     = create_string(response, res);
                        const auto& r = create<result_type>(response, str);
                        create<results_union>(response, r);
                    } else {
                        const auto& r = lidl::create<result_type>(response, res);
                        create<results_union>(response, r);
                    }
                }
            };

            apply(make_service_call, call_params);
        },
        get_root<params_union>(buffer));
}
} // namespace detail

template <class ServiceT>
procedure_runner_t make_procedure_runner() {
    return &detail::request_handler<ServiceT>;
}
} // namespace lidl