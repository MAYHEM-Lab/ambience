#include <doctest.h>
#include <tos/components/allocator.hpp>
#include <tos/components/threads.hpp>
#include <tos/ft.hpp>
#include <tos/allocator/null_allocator.hpp>
#include <tos/semaphore.hpp>

namespace tos {
namespace {
TEST_CASE("The initial thread starts in the default context") {
    REQUIRE_EQ(&default_context(), &current_context());
}

TEST_CASE("Setting the context of a thread works") {
    static_context<> c;
    self()->set_context(c);
    REQUIRE_EQ(&c, &current_context());
    self()->set_context(default_context());
}

TEST_CASE("Threads spawn in parent threads context") {
    static_context<> c;
    self()->set_context(c);
    tos::semaphore wait{0};
    tos::launch(tos::alloc_stack, [&] {
        REQUIRE_EQ(&c, &current_context());
        wait.up();
    });
    wait.down();
    self()->set_context(default_context());
}

TEST_CASE("Threads component keeps track of threads") {
    static_context<threads_component> c;
    auto& [threads] = c.all_components();

    // By default, there are no threads in the context.
    REQUIRE_EQ(0, threads.threads.size());

    // Move this thread to the new context.
    // There must be one thread in the context.
    self()->set_context(c);
    REQUIRE_EQ(1, threads.threads.size());

    // We will launch a new thread and make it block.
    // Since the new thread gets spawned in this thread's context, the context will have
    // 2 threads.
    tos::semaphore wait{0};
    tos::launch(tos::alloc_stack, [&] { wait.down(); });
    tos::this_thread::yield();
    REQUIRE_EQ(2, threads.threads.size());

    // Let the new thread exit. After that, the context should be back to 1 thread.
    wait.up();
    tos::this_thread::yield();
    REQUIRE_EQ(1, threads.threads.size());

    // Dissociate ourself from the context.
    // There is no thread left in the context.
    self()->set_context(default_context());
    REQUIRE_EQ(0, threads.threads.size());
}

TEST_CASE("Per context allocators work") {
    auto erased_alloc = memory::erase_allocator(memory::null_allocator{});
    static_context<allocator_component> c(erased_alloc);

    auto ptr = new int;
    REQUIRE_NE(nullptr, ptr);

    self()->set_context(c);

    auto ptr2 = new int;
    REQUIRE_EQ(nullptr, ptr2);

    self()->set_context(default_context());
}

TEST_CASE("Overlay contexts work") {
    static_context<threads_component> c;
    auto& [threads] = c.all_components();
    overlay_context overlay(c, current_context());

    // By default, there are no threads in the context.
    REQUIRE_EQ(0, threads.threads.size());

    // Move this thread to the new context.
    // There must be one thread in the context.
    self()->set_context(overlay);
    REQUIRE_EQ(1, threads.threads.size());

    auto ptr = new int;
    REQUIRE_NE(nullptr, ptr);

    self()->set_context(overlay.get_old());
}
} // namespace
} // namespace tos