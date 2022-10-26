//
// Created by fatih on 10/4/19.
//

#include "doctest.h"

#include <tos/ft.hpp>
#include <tos/mutex.hpp>

namespace tos {
namespace {
TEST_CASE("recursive mutex init") {
    recursive_mutex mutex;
    REQUIRE_FALSE(mutex.current_holder());
}

TEST_CASE("recursive mutex depth 1") {
    recursive_mutex mutex;
    mutex.lock();
    REQUIRE(mutex.current_holder());
    REQUIRE_EQ(tos::this_thread::get_id(), *mutex.current_holder());
    mutex.unlock();
    REQUIRE_FALSE(mutex.current_holder());
}

TEST_CASE("recursive mutex depth 2") {
    recursive_mutex mutex;
    mutex.lock();
    REQUIRE(mutex.current_holder());
    REQUIRE_EQ(tos::this_thread::get_id(), *mutex.current_holder());
    mutex.lock();
    REQUIRE(mutex.current_holder());
    REQUIRE_EQ(tos::this_thread::get_id(), *mutex.current_holder());
    mutex.unlock();
    REQUIRE(mutex.current_holder());
    REQUIRE_EQ(tos::this_thread::get_id(), *mutex.current_holder());
    mutex.unlock();
    REQUIRE_FALSE(mutex.current_holder());
}
} // namespace
} // namespace tos
