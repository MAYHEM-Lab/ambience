#include <doctest.h>
#include <tos/error.hpp>

namespace tos {
namespace {
struct my_err {
    std::string_view message() const {
        return "yo";
    }
    std::string_view name() const {
        return "my_err";
    }
};

TEST_CASE("error_cast works") {
    any_error err = my_err{};
    REQUIRE_EQ("yo", err.message());
    REQUIRE(error_cast<my_err>(err));
    REQUIRE_EQ("yo", error_cast<my_err>(err)->message());
}

enum class err_enum {
    foo,
    bar
};

enum_error<err_enum> fn() {
    return err_enum::bar;
}

TEST_CASE("enum error works") {
    auto err = fn();
    REQUIRE(err.name().ends_with("err_enum"));
    REQUIRE_EQ("bar", err.message());
}
} // namespace
} // namespace tos