#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

void tos_force_get_failed(void*) {
    std::terminate();
}

namespace tos::debug {
void default_assert_handler(const char* err) {
    std::cerr << err << '\n';
    std::terminate();
}
}