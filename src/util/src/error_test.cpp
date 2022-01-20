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
} // namespace
} // namespace tos