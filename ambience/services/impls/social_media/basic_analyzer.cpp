#include <analysis_generated.hpp>

namespace social_media {
namespace {
struct basic_analyzer : post_analysis::async_server {
    tos::Task<const lidl::vector<social_media::common::int_range>&>
    get_hashtags(std::string_view post,
                 lidl::message_builder& response_builder) override {
        std::vector<common::int_range> result;

        int length = post.size();

        for (int i = 1; i < length; i++) {
            if (post[i - 1] == ' ' &&
                post[i] == '#') { // Mention begins with '@' symbol preceded by space
                int offset = i;   // Token begins at '@' symbol (offset)
                while (post[i] != ' ' && i < length) {
                    i++; // Continue until end of mention (either space or end of content)
                }
                result.emplace_back(offset, i);
            }
        }

        co_return lidl::create_vector<common::int_range>(response_builder, result);
    }

    tos::Task<const lidl::vector<social_media::common::int_range>&>
    get_mentions(std::string_view post,
                 lidl::message_builder& response_builder) override {
        std::vector<common::int_range> result;

        int length = post.size();

        for (int i = 1; i < length; i++) {
            if (post[i - 1] == ' ' &&
                post[i] == '@') { // Mention begins with '@' symbol preceded by space
                int offset = i;   // Token begins at '@' symbol (offset)
                while (post[i] != ' ' && i < length) {
                    i++; // Continue until end of mention (either space or end of content)
                }
                result.emplace_back(offset, i);
            }
        }

        co_return lidl::create_vector<common::int_range>(response_builder, result);
    }
};
} // namespace
} // namespace social_media

tos::Task<social_media::post_analysis::async_server*> init_basic_analyzer() {
    co_return new social_media::basic_analyzer;
}