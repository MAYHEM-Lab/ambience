//
// Created by fatih on 1/24/20.
//

#pragma once

#include <cstring>
#include <lidlrt/builder.hpp>
#include <lidlrt/meta.hpp>
#include <lidlrt/status.hpp>
#include <string_view>
#include <tos/task.hpp>
#include <tuple>

namespace lidl {
template<class T>
class service_descriptor;

template<auto Fn, auto AsyncFn, class ParamsT, class ResultsT>
struct procedure_descriptor {
    static constexpr auto function = Fn;
    static constexpr auto async_function = AsyncFn;
    using params_type = ParamsT;
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
    virtual ~service_base() = default;
};

class sync_service_base : public service_base {};

class async_service_base : public service_base {};

template<class T>
tos::span<uint8_t> as_span(T&);

namespace meta {
template<class... ParamsT>
struct get_result_type_impl;

template<auto... Procs, auto... AsyncProcs, class... ParamsT, class... ResultsT>
struct get_result_type_impl<const std::tuple<
    lidl::procedure_descriptor<Procs, AsyncProcs, ParamsT, ResultsT>...>> {
    using params = std::tuple<ParamsT...>;
    using results = std::tuple<ResultsT...>;
};
} // namespace meta

template<class ServiceT>
using typed_async_procedure_runner_t = tos::Task<bool> (*)(ServiceT&,
                                                           tos::span<uint8_t>,
                                                           lidl::message_builder&);

template<class ServiceT>
using typed_procedure_runner_t = bool (*)(ServiceT&,
                                          tos::span<uint8_t>,
                                          lidl::message_builder&);

using async_erased_procedure_runner_t = typed_async_procedure_runner_t<service_base>;
using erased_procedure_runner_t = typed_procedure_runner_t<service_base>;

template<class ServiceT>
using typed_union_procedure_runner_t =
    bool (*)(ServiceT&,
             typename ServiceT::service_type::wire_types::call_union&,
             lidl::message_builder&);

template<class ServiceT>
using typed_async_union_procedure_runner_t =
    tos::Task<bool> (*)(ServiceT&,
                        typename ServiceT::service_type::wire_types::call_union&,
                        lidl::message_builder&);

template<class>
class print;

namespace detail {
template<class ServiceT, class BaseServT = ServiceT>
tos::Task<bool>
async_union_caller(BaseServT& base_service,
                   typename ServiceT::service_type::wire_types::call_union& call_union,
                   lidl::message_builder& response) {
    /**
     * Don't panic!
     *
     * Yes, this is full of fearsome metaprogramming magic, but I'll walk you through.
     */

    // The service descriptor stores the list of procedures in a service. We'll use
    // this information to decode lidl messages into actual calls to services.
    using descriptor = service_descriptor<typename ServiceT::service_type>;

    using results_union = typename descriptor::results_union;

    using all_params =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::params;
    using all_results =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::results;

    co_return co_await visit(
        [&service = static_cast<ServiceT&>(base_service),
         &response](auto& call_params) -> tos::Task<bool> {
            constexpr auto idx = meta::tuple_index_of<
                std::remove_const_t<std::remove_reference_t<decltype(call_params)>>,
                all_params>::value;
            using result_type =
                std::remove_const_t<std::remove_reference_t<decltype(std::get<idx>(
                    std::declval<all_results>()))>>;

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
            auto make_service_call = [&service,
                                      &response](auto&&... args) -> tos::Task<bool> {
                constexpr auto proc = rpc_param_traits<std::remove_const_t<
                    std::remove_reference_t<decltype(call_params)>>>::async_params_for;

                using proc_traits = procedure_traits<decltype(proc)>;
                if constexpr (!proc_traits::takes_response_builder()) {
                    auto res = co_await std::invoke(proc, service, args...);
                    create<results_union>(response, result_type(res));
                } else {
                    const auto& res =
                        co_await std::invoke(proc, service, args..., response);
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

                        auto& str = create_string(response, res);
                        const auto& r = create<result_type>(response, str);
                        create<results_union>(response, r);
                    } else if constexpr (std::is_same_v<meta::remove_cref<decltype(res)>,
                                                        tos::span<uint8_t>>) {
                        auto& str = create_vector(response, res);
                        const auto& r = create<result_type>(response, str);
                        create<results_union>(response, r);
                    } else {
                        const auto& r = lidl::create<result_type>(response, res);
                        create<results_union>(response, r);
                    }
                }

                co_return true;
            };

            co_return co_await apply(make_service_call, call_params);
        },
        call_union);
}

template<class ServiceT, class BaseServT = ServiceT>
bool union_caller(BaseServT& base_service,
                  typename ServiceT::service_type::wire_types::call_union& call_union,
                  lidl::message_builder& response) {
    /**
     * Don't panic!
     *
     * Yes, this is full of fearsome metaprogramming magic, but I'll walk you through.
     */

    // The service descriptor stores the list of procedures in a service. We'll use
    // this information to decode lidl messages into actual calls to services.
    using descriptor = service_descriptor<typename ServiceT::service_type>;

    using results_union = typename descriptor::results_union;

    using all_params =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::params;
    using all_results =
        typename meta::get_result_type_impl<decltype(descriptor::procedures)>::results;

    return visit(
        [&service = static_cast<ServiceT&>(base_service),
         &response](auto& call_params) -> decltype(auto) {
            constexpr auto idx = meta::tuple_index_of<
                std::remove_const_t<std::remove_reference_t<decltype(call_params)>>,
                all_params>::value;
            using result_type =
                std::remove_const_t<std::remove_reference_t<decltype(std::get<idx>(
                    std::declval<all_results>()))>>;

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
            auto make_service_call = [&service, &response](auto&&... args) -> bool {
                constexpr auto proc = rpc_param_traits<std::remove_const_t<
                    std::remove_reference_t<decltype(call_params)>>>::params_for;

                using proc_traits = procedure_traits<decltype(proc)>;
                if constexpr (!proc_traits::takes_response_builder()) {
                    auto res = std::invoke(proc, service, args...);
                    create<results_union>(response, result_type(res));
                } else {
                    const auto& res = std::invoke(proc, service, args..., response);
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

                        auto& str = create_string(response, res);
                        const auto& r = create<result_type>(response, str);
                        create<results_union>(response, r);
                    } else if constexpr (std::is_same_v<meta::remove_cref<decltype(res)>,
                                                        tos::span<uint8_t>>) {
                        auto& str = create_vector(response, res);
                        const auto& r = create<result_type>(response, str);
                        create<results_union>(response, r);
                    } else {
                        const auto& r = lidl::create<result_type>(response, res);
                        create<results_union>(response, r);
                    }
                }

                return true;
            };

            return apply(make_service_call, call_params);
        },
        call_union);
}

template<class ServiceT, class BaseServT = ServiceT>
tos::Task<bool> async_request_handler(BaseServT& base_service,
                                      tos::span<uint8_t> buffer,
                                      lidl::message_builder& response) {
    static_assert(std::is_base_of_v<BaseServT, ServiceT>);
    using descriptor = service_descriptor<typename ServiceT::service_type>;

    using params_union = typename descriptor::params_union;

    return async_union_caller<ServiceT, BaseServT>(
        base_service, get_root<params_union>(buffer), response);
}

template<class ServiceT, class BaseServT = ServiceT>
bool request_handler(BaseServT& base_service,
                     tos::span<uint8_t> buffer,
                     lidl::message_builder& response) {
    static_assert(std::is_base_of_v<BaseServT, ServiceT>);
    using descriptor = service_descriptor<typename ServiceT::service_type>;

    using params_union = typename descriptor::params_union;

    return union_caller<ServiceT, BaseServT>(
        base_service, get_root<params_union>(buffer), response);
}
} // namespace detail

template<class ServiceT, class BaseServiceT = ServiceT>
typed_procedure_runner_t<BaseServiceT> make_procedure_runner() {
    return &detail::request_handler<ServiceT, BaseServiceT>;
}

template<class ServiceT>
erased_procedure_runner_t make_erased_procedure_runner() {
    return &detail::request_handler<ServiceT, service_base>;
}

template<class ServiceT>
async_erased_procedure_runner_t make_async_erased_procedure_runner() {
    return &detail::async_request_handler<ServiceT, service_base>;
}


template<class ServiceT, class BaseServiceT = ServiceT>
typed_union_procedure_runner_t<BaseServiceT> make_union_procedure_runner() {
    return &detail::union_caller<ServiceT, BaseServiceT>;
}
} // namespace lidl