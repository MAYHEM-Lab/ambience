#pragma once

#include <lidl/lidl.hpp>
#include <lidl/service.hpp>
#include <lidl/string.hpp>
#include <type_traits>

template<class... ParamsT>
struct get_result_type_impl;

template<auto... Procs, class... ParamsT, class... ResultsT>
struct get_result_type_impl<
    const std::tuple<lidl::procedure_descriptor<Procs, ParamsT, ResultsT>...>> {
    using params = std::tuple<ParamsT...>;
    using results = std::tuple<ResultsT...>;
};

template<class ServiceT, class ParamsT>
struct get_result_type {};

template<class T, class Tuple>
struct Index;

template<class T, class... Types>
struct Index<T, std::tuple<T, Types...>> {
    static const std::size_t value = 0;
};

template<class T, class U, class... Types>
struct Index<T, std::tuple<U, Types...>> {
    static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

template<class T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;

template<class ServiceT>
void request_handler(ServiceT& service,
                     lidl::buffer buffer,
                     lidl::message_builder& response) {
    /**
     * Don't panic!
     *
     * Yes, this is full of fearsome metaprogramming magic, but I'll walk you through.
     */

    // The service descriptor stores the list of procedures in a service. We'll use
    // this information to decode lidl messages into actual calls to services.
    using descriptor = lidl::service_descriptor<ServiceT>;

    using params_union = typename ServiceT::call_union;
    using results_union = typename ServiceT::return_union;

    using all_params =
        typename get_result_type_impl<decltype(descriptor::procedures)>::params;
    using all_results =
        typename get_result_type_impl<decltype(descriptor::procedures)>::results;

    visit(
        [&](const auto& call_params) -> decltype(auto) {
            constexpr auto proc = lidl::rpc_param_traits<std::remove_const_t<
                std::remove_reference_t<decltype(call_params)>>>::params_for;
            using proc_traits = lidl::procedure_traits<decltype(proc)>;
            constexpr auto idx =
                Index<std::remove_const_t<std::remove_reference_t<decltype(call_params)>>,
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
              constexpr auto proc = lidl::rpc_param_traits<std::remove_const_t<
                  std::remove_reference_t<decltype(call_params)>>>::params_for;

              if constexpr (!proc_traits::takes_response_builder()) {
                    auto res = (service.*(proc))(args...);
                    lidl::create<results_union>(response, result_type(res));
                } else {
                    const auto& res = (service.*(proc))(args..., response);
                    if constexpr (std::is_same_v<remove_cref<decltype(res)>,
                                                 std::string_view>) {
                        /**
                         * The procedure returned a view.
                         *
                         * We need to see if the returned view is already in the
                         * response buffer. If it is not, we will copy it.
                         *
                         * Issue #6.
                         */

                        auto& str = lidl::create_string(response, res);
                        const auto& r = lidl::create<result_type>(response, str);
                        lidl::create<results_union>(response, r);
                    } else {
                        const auto& r = lidl::create<result_type>(response, res);
                        lidl::create<results_union>(response, r);
                    }
                }
            };

            lidl::apply(make_service_call, call_params);
        },
        lidl::get_root<params_union>(buffer));
}

template<class ServiceT>
auto make_request_handler() {
    return &request_handler<ServiceT>;
}
