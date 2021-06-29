#include <block_memory_generated.hpp>

namespace {
class impl : public tos::ae::services::block_memory::sync_server {
public:
    impl() = default;

    int32_t get_block_size() override {
        return 512;
    }

    int32_t get_block_count() override {
        return 128;
    }

    tos::span<uint8_t> read(const int32_t& block,
                            const int32_t& offset,
                            const int32_t& len,
                            ::lidl::message_builder& response_builder) override {
        auto start = response_builder.allocate(len, 1);
        auto res = tos::span<uint8_t>(start, len);
        tos::safe_span_copy(res, span().slice(get_block_size() * block + offset, len));
        return res;
    }

    bool erase(const int32_t& block) override {
        auto buf = span().slice(get_block_size() * block, get_block_size());
        std::fill(buf.begin(), buf.end(), -1);
        return true;
    }

    bool
    write(const int32_t& block, const int32_t& offset, tos::span<uint8_t> data) override {
        auto buf = span().slice(get_block_size() * block + offset, data.size());
        tos::safe_span_copy(buf, data);
        return true;
    }

    tos::span<uint8_t> span() {
        return m_data;
    }

    std::vector<uint8_t> m_data = std::vector<uint8_t>(128 * 512);
};
class async_impl : public tos::ae::services::block_memory::async_server {
public:
    async_impl() = default;

    tos::Task<int32_t> get_block_size() override {
        co_return 512;
    }
    tos::Task<int32_t> get_block_count() override {
        co_return 128;
    }

    tos::Task<tos::span<uint8_t>>
    read(const int32_t& block,
         const int32_t& offset,
         const int32_t& len,
         ::lidl::message_builder& response_builder) override {
        auto start = response_builder.allocate(len, 1);
        auto res = tos::span<uint8_t>(start, len);
        tos::safe_span_copy(
            res, span().slice(co_await get_block_size() * block + offset, len));
        co_return res;
    }

    tos::Task<bool> erase(const int32_t& block) override {
        auto buf =
            span().slice(co_await get_block_size() * block, co_await get_block_size());
        std::fill(buf.begin(), buf.end(), -1);
        co_return true;
    }

    tos::Task<bool>
    write(const int32_t& block, const int32_t& offset, tos::span<uint8_t> data) override {
        auto buf = span().slice(co_await get_block_size() * block + offset, data.size());
        tos::safe_span_copy(buf, data);
        co_return true;
    }

    tos::span<uint8_t> span() {
        return m_data;
    }

    std::vector<uint8_t> m_data = std::vector<uint8_t>(128 * 512);
};
} // namespace

tos::ae::services::block_memory::sync_server* init_ephemeral_block() {
    return new impl();
}

tos::ae::services::block_memory::async_server* init_async_ephemeral_block() {
    return new async_impl();
}