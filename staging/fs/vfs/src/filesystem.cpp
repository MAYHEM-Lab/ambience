#include <tos/vfs/filesystem.hpp>

namespace tos::vfs {
expected<node_ptr, errors> filesystem::open(node_id_t id, node_mode mode) {
    return do_open(id, mode);
}
}