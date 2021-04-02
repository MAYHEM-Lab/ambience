#include <doctest.h>
#include <tos/components/allocator.hpp>
#include <tos/components/name.hpp>
#include <tos/context.hpp>
#include <tos/ft.hpp>
#include <tos/allocator/malloc.hpp>

namespace tos {
namespace {
TEST_CASE("Static context works with a single component") {
    auto erased = tos::memory::erase_allocator(memory::mallocator{});
    static_context<allocator_component> ctx(erased);
    auto ptr = ctx.get_component<allocator_component>();
    REQUIRE_NE(nullptr, ptr);
}

TEST_CASE("Static context works with multiple components") {
    auto erased = tos::memory::erase_allocator(memory::mallocator{});
    static_context<allocator_component, name_component> ctx(erased, name_component{});
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
    REQUIRE_EQ(nullptr, current_context().get_component<name_component>());
    self()->set_context(ctx);
    REQUIRE_EQ("Test Context", current_context().get_component<name_component>()->name);
    self()->set_context(default_context());
}
} // namespace
} // namespace tos