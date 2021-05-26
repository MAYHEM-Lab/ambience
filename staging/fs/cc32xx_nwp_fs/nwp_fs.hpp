#pragma once

#include <tos/vfs/filesystem.hpp>
#include <tos/expected.hpp>

namespace tos::cc32xx {
expected<vfs::fs_ptr, vfs::errors> mount_nwp_fs();
}