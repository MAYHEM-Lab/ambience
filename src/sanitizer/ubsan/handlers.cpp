#include <tos/sanitizer/ubsan.hpp>
#include <cstdint>

namespace tos::ubsan::handlers {
void nonnull_arg() {
    while (true);
}

void load_invalid_value() {
    while (true);
}
}

extern "C" {
void __ubsan_handle_nonnull_arg() {
    tos::ubsan::handlers::nonnull_arg();
}
void __ubsan_handle_load_invalid_value(int32_t, int32_t) {
    tos::ubsan::handlers::nonnull_arg();
}
}