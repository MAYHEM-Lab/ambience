#include "lidlrt/string.hpp"
#include <echo_generated.hpp>

namespace {
struct impl : tos::ae::services::echo::sync_server {
    int32_t echo_num(const int32_t& num) override {
        return num;
    }

    std::string_view echo_str(std::string_view data,
                              lidl::message_builder& response_builder) override {
        return data;
    }
};

struct async_impl : tos::ae::services::echo::async_server {
    tos::Task<int32_t> echo_num(const int32_t& num) override {
        co_return num;
    }

    tos::Task<std::string_view>
    echo_str(std::string_view data, lidl::message_builder& response_builder) override {
        co_return lidl::create_string(response_builder, data).string_view();
    }
};
} // namespace

tos::ae::services::echo::sync_server* init_basic_echo() {
    return new impl();
}

tos::Task<tos::ae::services::echo::async_server*> init_async_basic_echo() {
    co_return new async_impl();
}