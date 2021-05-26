#pragma once

#include <cstdint>
#include <tos/intrusive_ptr.hpp>

namespace tos::vfs {
enum class error_code {
    not_supported,
    bad_mode,
    not_found
};

struct errors {
    error_code code;
};

using node_id_t = uint32_t;
using file_size_t = uint32_t;
using file_offset_t = uint32_t;

enum class node_type : uint8_t {
    file,
    directory,
};

enum class node_mode : uint8_t {
    readable = 1,
    writeable = 2,
    read_write = 3
};

class node;
class file;
class directory;

class filesystem;

using node_ptr = intrusive_ptr<node>;
using fs_ptr = intrusive_ptr<filesystem>;
}