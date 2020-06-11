//
// Created by fatih on 1/24/20.
//

#pragma once

#include <lidl/meta.hpp>
#include <lidl/status.hpp>
#include <string_view>
#include <tuple>
#include <lidl/builder.hpp>

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
        return (... || std::is_same_v<std::remove_reference_t<ArgTypes>, message_builder>);
    }
};

template<class Type, class RetType, class... ArgTypes>
struct procedure_traits<RetType (Type::*)(ArgTypes...)> : procedure_traits<RetType (Type::*const)(ArgTypes...)> {
};

template <class ServiceT>
class service_call_union;

template <class ServiceT>
class service_return_union;

template <class T>
class rpc_param_traits;

class service_base {
public:
    virtual ~service_base() = default;
    virtual std::string_view name() const = 0;
};

template <class ServiceT>
class service : public service_base {
public:
    std::string_view name() const override {
        return service_descriptor<ServiceT>::name;
    }

    using call_union = service_call_union<ServiceT>;
    using return_union = service_return_union<ServiceT>;
};
} // namespace lidl