#include <tos/ae/detail/syscall.hpp>

[[clang::import_module("env"), clang::import_name("_external_init_syscall")]]
void _external_init_syscall(tos::ae::interface& iface);

[[clang::import_module("env"), clang::import_name("_external_yield_syscall")]]
void _external_yield_syscall();

namespace tos::ae::detail {
void do_init_syscall(interface& iface) {
    _external_init_syscall(iface);
}
void do_yield_syscall() {
    _external_yield_syscall();
}
} // namespace tos::ae::detail