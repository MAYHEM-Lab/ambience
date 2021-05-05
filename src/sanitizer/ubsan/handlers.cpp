#include <tos/sanitizer/ubsan.hpp>
#include <cstdint>

extern "C" {
void __ubsan_handle_nonnull_arg() {
    tos::ubsan::handlers::handlers->error();
}
void __ubsan_handle_load_invalid_value(int32_t, int32_t) {
    tos::ubsan::handlers::handlers->error();
}
void __ubsan_handle_type_mismatch_v1() {
    tos::ubsan::handlers::handlers->error();
}
}