#include <doctest.h>
#include <tos/context.hpp>
#include <tos/ft.hpp>
#include <tos/components/allocator.hpp>
#include <tos/components/name.hpp>

namespace tos {
namespace {
TEST_CASE("Static context works with a single component") {
    static_context<allocator_component> ctx;
    auto ptr = ctx.get_component<allocator_component>();
    REQUIRE_NE(nullptr, ptr);
}

TEST_CASE("Static context works with multiple components") {
    static_context<allocator_component, name_component> ctx;
    auto ptr = ctx.get_component<allocator_component>();
    REQUIRE_NE(nullptr, ptr);

    auto name_ptr = ctx.get_component<name_component>();
    REQUIRE_NE(nullptr, name_ptr);
}

TEST_CASE("Thread context setting works") {
    static_context<name_component> ctx;
    auto name_ptr = ctx.get_component<name_component>();
    REQUIRE_NE(nullptr, name_ptr);
    name_ptr->name = "Test Context";
    // By default, each thread belongs to the default context, which does not have a name
    REQUIRE_EQ(nullptr, current_context()->get_component<name_component>());
    self()->m_context = &ctx;
    REQUIRE_EQ("Test Context", current_context()->get_component<name_component>()->name);
    self()->m_context = nullptr;
}
} // namespace
} // namespace tos