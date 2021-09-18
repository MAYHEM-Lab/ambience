#include <tos/sanitizer/ubsan.hpp>
#include <cstdint>

extern "C" {
void __ubsan_handle_nonnull_arg(void*) {
    tos::ubsan::handlers::handlers->error();
}
void __ubsan_handle_load_invalid_value(void*, void*) {
    tos::ubsan::handlers::handlers->error();
}
void __ubsan_handle_type_mismatch_v1(void*, void*) {
    tos::ubsan::handlers::handlers->error();
}
}