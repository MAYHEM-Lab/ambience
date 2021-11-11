#include <functional>
#include <kvstore_generated.hpp>
#include <lidlrt/find_extent.hpp>
#include <string_view>
#include <tos/compiler.hpp>
#include <tos/debug/log.hpp>
#include <unordered_map>

namespace tos::ae::services {
namespace {
template<class T>
struct cell {
    explicit cell(const T& t) {
        auto [extent, pos] = lidl::meta::detail::find_extent_and_position(t);
        m_storage = std::make_unique<uint8_t[]>(extent.size());
        memcpy(m_storage.get(), extent.data(), extent.size());
        m_elem = reinterpret_cast<const T*>(m_storage.get() + pos);
    }

    const T& get() const {
        return *m_elem;
    }

    const T* m_elem;
    std::unique_ptr<uint8_t[]> m_storage;
};

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

struct impl : KVStore::async_server {
    cell<Value>* get(std::string_view key) {
        if (auto it = m_table.find(key); it != m_table.end()) {
            return &it->second;
        }
        return nullptr;
    }

    tos::Task<const Value&> get(std::string_view key,
                                ::lidl::message_builder& response_builder) override {
        auto val = get(key);
        if (!val) {
            tos::debug::log("wtf");
            while (true)
                ;
            TOS_UNREACHABLE();
        }
        auto [ext, off] = lidl::meta::detail::find_extent_and_position(val->get());
        tos::debug::log(key, ext);
        auto ptr = response_builder.allocate(ext.size(), 1);
        memcpy(ptr, ext.data(), ext.size_bytes());
        auto obj_ptr = reinterpret_cast<const Value*>(ptr + off);
        co_return *obj_ptr;
    }

    tos::Task<bool> has(std::string_view key) override {
        co_return m_table.find(key) != m_table.end();
    }

    tos::Task<bool> put(std::string_view key, const Value& val) override {
        auto [it, res] = m_table.emplace(std::string(key), cell(val));
        tos::debug::log(key, lidl::meta::detail::find_extent(val));
        co_return res;
    }

    std::unordered_map<std::string, cell<Value>, string_hash, std::equal_to<>> m_table;
};

struct bytestore : ByteStore::async_server {
    std::optional<span<uint8_t>> get(std::string_view key) {
        if (auto it = m_table.find(key); it != m_table.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    tos::Task<span<uint8_t>> get(std::string_view key,
                                 ::lidl::message_builder& response_builder) override {
        auto val = get(key);
        if (!val) {
            co_return span<uint8_t>{};
        }
        co_return *val;
    }

    tos::Task<bool> has(std::string_view key) override {
        co_return m_table.find(key) != m_table.end();
    }

    tos::Task<bool> put(std::string_view key, span<uint8_t> val) override {
        auto [it, res] = m_table.emplace(std::string(key),
                                         std::vector<uint8_t>(val.begin(), val.end()));
        co_return res;
    }

    tos::Task<bool> erase(std::string_view key) override {
        auto it = m_table.find(key);
        if (it == m_table.end()) {
            co_return false;
        }
        m_table.erase(it);
        co_return true;
    }

    std::unordered_map<std::string, std::vector<uint8_t>, string_hash, std::equal_to<>>
        m_table;
};
} // namespace
} // namespace tos::ae::services

tos::Task<tos::ae::services::KVStore::async_server*> init_inmem_kv() {
    co_return new tos::ae::services::impl;
}

tos::Task<tos::ae::services::ByteStore::async_server*> init_inmem_bytestore() {
    co_return new tos::ae::services::bytestore;
}