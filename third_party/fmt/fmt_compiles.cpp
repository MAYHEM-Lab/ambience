#include <fmt/core.h>

std::string foo() {
    return fmt::format("hello {}", "world");
}