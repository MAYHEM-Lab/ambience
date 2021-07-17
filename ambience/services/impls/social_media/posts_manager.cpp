#include <analysis_generated.hpp>
#include <posts_generated.hpp>
#include <unordered_map>

namespace social_media {
namespace {
struct posts_manager : posts::async_server {
    explicit posts_manager(social_media::post_analysis::async_server* analysis)
        : m_analysis{analysis} {
    }

    tos::Task<const social_media::common::post&>
    get_post(const uint64_t& id, lidl::message_builder& response_builder) override {
        auto& post = m_posts[id];

        co_return lidl::create<common::post>(
            response_builder,
            lidl::create_string(response_builder, post.body),
            lidl::create_string(response_builder, post.user),
            0,
            lidl::create_vector<common::int_range>(response_builder),
            lidl::create_vector<common::int_range>(response_builder));
    }

    tos::Task<uint64_t> send_post(std::string_view user, std::string_view body) override {
        auto res = m_posts.emplace(
            m_next++, post_data{.user = std::string(user), .body = std::string(body)});
        co_return res.first->first;
    }


    struct post_data {
        std::string user;
        std::string body;
    };

    uint64_t m_next;
    std::unordered_map<uint64_t, post_data> m_posts;
    social_media::post_analysis::async_server* m_analysis;
};
} // namespace
} // namespace social_media

tos::Task<social_media::posts::async_server*>
init_posts_manager(social_media::post_analysis::async_server* analysis) {
    co_return new social_media::posts_manager{analysis};
}