#include <tos/vfs/directory.hpp>

namespace tos::vfs {
expected<void, errors> directory::add_entry(std::string_view name, const node& node) {
    if (!util::is_flag_set(mode(), node_mode::writeable)) {
        return unexpected(errors{error_code::bad_mode});
    }

    return do_add_entry(name, node);
}

expected<void, errors> directory::remove_entry(std::string_view name, const node& node) {
    if (!util::is_flag_set(mode(), node_mode::writeable)) {
        return unexpected(errors{error_code::bad_mode});
    }

    return do_remove_entry(name, node);
}
} // namespace tos::vfs