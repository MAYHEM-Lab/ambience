//
// Created by fatih on 10/4/19.
//

#include "doctest.h"
#include <tos/mutex.hpp>

namespace tos {
namespace {
TEST_CASE ("basic recursive mutex") {
    recursive_mutex mutex;
    mutex.lock();
    mutex.lock();
    REQUIRE(true);
}
}
}