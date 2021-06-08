#include <ebpf_test_generated.hpp>
#include <tos/flags.hpp>

namespace {
bool starts_with(std::string_view a, std::string_view b) {
    return a.substr(0, std::min(a.size(), b.size())) == b;
}
} // namespace

bool check_path_constraint(tos::path_capability* pc) {
    return starts_with(pc->path(), "/home/meow") &&
           !tos::util::is_flag_set(pc->perm(), tos::permissions::execute);
}