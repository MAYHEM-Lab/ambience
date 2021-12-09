#include "lidlrt/string.hpp"
#include "tos/detail/coro.hpp"
#include <dns_generated.hpp>
#include <string>
#include <string_view>
#include <tos/coro/for_each.hpp>
#include <unordered_map>

namespace tos::ae::services {
namespace {
struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
    size_t operator()(std::string_view txt) const {
        return hash_type{}(txt);
    }
    size_t operator()(const std::string& txt) const {
        return hash_type{}(txt);
    }
    size_t operator()(const char* txt) const {
        return hash_type{}(txt);
    }
};

struct final_dns_resolver : dns::async_server {
    tos::Task<std::string_view>
    resolve(std::string_view hostname,
            std::string_view type,
            ::lidl::message_builder& response_builder) override {
        if (auto it = m_cache.find(hostname); it != m_cache.end()) {
            co_return lidl::create_string(response_builder, it->second);
        }

        co_return "";
    }

    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> m_cache{
        {"lidl.dev", "10.0.0.1"},
        {"google.com", "1.2.3.4"},
    };
};

struct final_dns_resolver2 : dns::async_server {
    tos::Task<std::string_view>
    resolve(std::string_view hostname,
            std::string_view type,
            ::lidl::message_builder& response_builder) override {
        if (auto it = m_cache.find(hostname); it != m_cache.end()) {
            co_return lidl::create_string(response_builder, it->second);
        }

        co_return "";
    }

    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> m_cache{
        {"example.com", "8.8.8.8"},
        {"a6e.org", "123.234.231.123"},
    };
};

struct async_recursive_dns_resolver : dns::async_server {
    async_recursive_dns_resolver(tos::ae::services::dns::async_server* b1,
                                 tos::ae::services::dns::async_server* b2)
        : backends{b1, b2} {
    }

    tos::Task<std::string_view>
    resolve(std::string_view hostname,
            std::string_view type,
            ::lidl::message_builder& response_builder) override {
        if (auto it = m_cache.find(hostname); it != m_cache.end()) {
            co_return lidl::create_string(response_builder, it->second);
        }

        for (auto backend : backends) {
            if (auto res = co_await backend->resolve(hostname, type, response_builder);
                !res.empty()) {
                m_cache.emplace(std::string(hostname), std::string(res));
                co_return res;
            }
        }

        co_return "";
    }

    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> m_cache;
    std::vector<dns::async_server*> backends;
};

struct sync_recursive_dns_resolver : dns::sync_server {
    sync_recursive_dns_resolver(tos::ae::services::dns::sync_server* b1,
                                tos::ae::services::dns::sync_server* b2)
        : backends{b1, b2} {
    }

    std::string_view resolve(std::string_view hostname,
                             std::string_view type,
                             ::lidl::message_builder& response_builder) override {
        if (auto it = m_cache.find(hostname); it != m_cache.end()) {
            return lidl::create_string(response_builder, it->second);
        }

        for (auto backend : backends) {
            if (auto res = backend->resolve(hostname, type, response_builder);
                !res.empty()) {
                m_cache.emplace(std::string(hostname), std::string(res));
                return res;
            }
        }

        return "";
    }

    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> m_cache;
    std::vector<dns::sync_server*> backends;
};
} // namespace
} // namespace tos::ae::services

tos::Task<tos::ae::services::dns::async_server*> init_final_dns_resolver() {
    co_return new tos::ae::services::final_dns_resolver;
}

tos::Task<tos::ae::services::dns::async_server*> init_final_dns_resolver2() {
    co_return new tos::ae::services::final_dns_resolver2;
}

tos::Task<tos::ae::services::dns::async_server*>
init_async_recursive_dns_resolver(tos::ae::services::dns::async_server* b1,
                                  tos::ae::services::dns::async_server* b2) {
    co_return new tos::ae::services::async_recursive_dns_resolver{b1, b2};
}

tos::ae::services::dns::sync_server*
init_sync_recursive_dns_resolver(tos::ae::services::dns::sync_server* b1,
                                 tos::ae::services::dns::sync_server* b2) {
    return new tos::ae::services::sync_recursive_dns_resolver{b1, b2};
}