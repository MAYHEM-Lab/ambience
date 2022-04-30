#include <calc_generated.hpp>
#include <echo_generated.hpp>
#include <doctest.h>
#include <jsmn.hpp>
#include <jsmn_lidl.hpp>

namespace jsmn {
namespace {
constexpr std::string_view json =
    R"__({"foo": {"bar": 42, "yolo": {"yo": {}}}, "zoo": "hello world"})__";
constexpr std::string_view calc_json = R"__({"index": 0, "x": 42, "y": 53})__";
constexpr std::string_view echo_json = R"__({"index": 1, "data": "yolooo"})__";

TEST_CASE("Jsmn parsing works") {
    auto toks = parse(json).value();

    parser p{
        .tokens = toks,
        .body = json,
    };

    REQUIRE(p.front().type == JSMN_OBJECT);

    object_parser obj_parse{p};

    REQUIRE_EQ(2, obj_parse.size());

    REQUIRE(obj_parse.has_key("foo"));
    REQUIRE(obj_parse.has_key("zoo"));
    REQUIRE(obj_parse.value("zoo").has_value());
}

TEST_CASE("Jsmn to lidl works") {
    auto toks = parse(calc_json).value();

    parser p{
        .tokens = toks,
        .body = calc_json,
    };

    REQUIRE(p.front().type == JSMN_OBJECT);

    object_parser obj_parse{p};

    std::vector<uint8_t> buf(1024);
    lidl::message_builder mb(buf);

    jmp_buf err;
    if (auto val = setjmp(err); val != 0) {
        REQUIRE_EQ(0, val);
        return;
    }

    auto call = tos::ae::detail::try_translate<
        tos::ae::services::calculator::wire_types::call_union>(obj_parse, mb, err);

    REQUIRE_EQ(tos::ae::services::calculator::wire_types::call_union::alternatives::add,
               call.alternative());
    REQUIRE_EQ(42, call.add().x());
    REQUIRE_EQ(53, call.add().y());
}

TEST_CASE("Jsmn to lidl works with strings") {
    auto toks = parse(echo_json).value();

    parser p{
        .tokens = toks,
        .body = echo_json,
    };

    REQUIRE(p.front().type == JSMN_OBJECT);

    object_parser obj_parse{p};

    std::vector<uint8_t> buf(1024);
    lidl::message_builder mb(buf);

    jmp_buf err;
    if (auto val = setjmp(err); val != 0) {
        REQUIRE_EQ(0, val);
        return;
    }

    const auto& call = tos::ae::detail::try_translate<
        tos::ae::services::echo::wire_types::call_union>(obj_parse, mb, err);

    REQUIRE_EQ(tos::ae::services::echo::wire_types::call_union::alternatives::echo_str,
               call.alternative());
    REQUIRE_EQ("yolooo", call.echo_str().data().string_view());
}
} // namespace
} // namespace jsmn