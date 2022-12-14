#pragma once

#include "tos/task.hpp"
#include <lidlrt/service.hpp>
#include <string_view>
#include <tos/async_init.hpp>
#include <tos/debug/log.hpp>
#include <tos/fixed_string.hpp>

namespace tos::ae {
template<tos::fixed_string Name, class T>
struct service_mapping {
    tos::async_init<T> serv;

    void set_serv(T t) {
        serv.set_value(t);
    }

    auto* try_take() {
        return serv.has_value() ? &serv.value() : nullptr;
    }

    auto& take() {
        return serv.value();
    }

    auto wait() {
        return serv.operator co_await();
    }
};

struct registry_base {
public:
    virtual bool register_service(std::string_view name, lidl::service_base* serv) = 0;

    virtual lidl::service_base* try_take(std::string_view name) = 0;
    virtual tos::Task<lidl::service_base*> wait(std::string_view name) = 0;

    template<class T>
    tos::Task<T*> wait(std::string_view name) {
        co_return static_cast<T*>(co_await wait(name));
    }

    virtual ~registry_base() = default;
};

template<class...>
struct service_registry;

template<tos::fixed_string... Names, class... Ts>
struct service_registry<service_mapping<Names, Ts>...>
    : registry_base
    , service_mapping<Names, Ts>... {
    template<tos::fixed_string Name, class T>
    bool register_service(T serv) {
        get<Name>(*this).set_serv(serv);
        tos::debug::log("Registered service:", Name);
        return true;
    }

    template<tos::fixed_string Name>
    auto wait() {
        return get<Name>(*this).wait();
    }

    template<tos::fixed_string Name>
    auto& take() {
        return get<Name>(*this).take();
    }

    template<tos::fixed_string Name>
    auto* try_take() {
        return get<Name>(*this).try_take();
    }

    bool register_service(std::string_view name, lidl::service_base* serv) override {
        return ((std::string_view(Names) == name &&
                 register_service<Names>(static_cast<Ts>(serv))) ||
                ...);
    }

    lidl::service_base* try_take(std::string_view name) override {
        auto call_if = [name](auto&& x) {
            ((std::string_view(Names) == name ? x.template operator()<Names>() : false),
             ...);
        };

        lidl::service_base* ptr = nullptr;

        call_if([&]<tos::fixed_string Name>() {
            if (auto res = try_take<Name>()) {
                ptr = *res;
            }
            return true;
        });

        return ptr;
    }

    tos::Task<lidl::service_base*> wait(std::string_view name) override {
        auto call_if = [name](auto&& x) -> tos::Task<void> {
            ((std::string_view(Names) == name ? co_await x.template operator()<Names>()
                                              : false),
             ...);
        };

        lidl::service_base* ptr = nullptr;

        call_if([&]<tos::fixed_string Name>() -> tos::Task<bool> {
            ptr = co_await wait<Name>();
            co_return true;
        });

        co_return ptr;
    }

    template<tos::fixed_string Name>
    constexpr bool has_service() {
        return ((Names == Name) || ...);
    }

private:
    template<tos::fixed_string Name, class T>
    friend auto& get(service_mapping<Name, T>& mapping) {
        return mapping;
    }
};
} // namespace tos::ae