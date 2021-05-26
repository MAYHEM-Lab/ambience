#include <tos/flags.hpp>
#include <tos/vfs/file.hpp>

namespace tos::vfs {
expected<span<uint8_t>, errors> file::read(file_offset_t offset,
                                           tos::span<uint8_t> data) {
    if (!util::is_flag_set(mode(), node_mode::readable)) {
        return unexpected(errors{error_code::bad_mode});
    }

    return do_read(offset, data);
}

expected<size_t, errors> file::write(file_offset_t offset, span<const uint8_t> data) {
    if (!util::is_flag_set(mode(), node_mode::writeable)) {
        return unexpected(errors{error_code::bad_mode});
    }

    return do_write(offset, data);
}
} // namespace tos::vfs