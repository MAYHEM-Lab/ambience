#include <cstring>
#include <tos/compiler.hpp>
#include <tos/debug/log.hpp>
#include <tos/span.hpp>
#include <tos/vfs/directory.hpp>
#include <tos/vfs/file.hpp>
#include <tos/vfs/filesystem.hpp>

namespace tos::mappedromfs {
namespace {
static constexpr uint8_t magic_bytes[4] = {0x66, 0xFF, 0x66, 0xBB};
using vfs::errors;
using vfs::node_id_t;
struct PACKED fs_header {
    uint8_t magic_bytes[4];
    uint32_t size;
};

struct PACKED file_header {
    // The highest byte determines if the file is a file or a directory.
    // If it's 0, it's a directory.
    uint32_t type_and_size;
};

class filesystem : public vfs::filesystem {
public:
    explicit filesystem(span<const uint8_t> data)
        : vfs::filesystem(true)
        , m_data{data} {
    }

    void collect_node(vfs::node* obj) override {
        delete obj;
    }

    node_id_t root_node_id() override {
        return sizeof(fs_header);
    }

private:
    void do_sync() override {
    }

    expected<vfs::node_ptr, errors> do_open(node_id_t id, vfs::node_mode mode) override;

public:
    span<const uint8_t> m_data;
};

class file : public vfs::file {
    vfs::file_size_t size() const override {
        return file_body().size();
    }

    expected<span<uint8_t>, errors> do_read(vfs::file_offset_t offset,
                                            span<uint8_t> data) override {
        return safe_span_copy(data, file_body().slice(offset));
    }

    expected<size_t, errors> do_write(vfs::file_offset_t offset,
                                      span<const uint8_t> data) override {
        return unexpected(errors{vfs::error_code::not_supported});
    }

public:
    span<const uint8_t> entire_file() const {
        auto start = static_cast<const filesystem*>(fs())->m_data.slice(id());
        uint32_t size;
        memcpy(&size, start.data(), 4);
        size &= 0x7F'FF'FF'FFU;
        return start.slice(0, size + 4);
    }

    span<const uint8_t> file_body() const {
        return entire_file().slice(sizeof(file_header));
    }

    file(vfs::fs_ptr fs, node_id_t id)
        : vfs::file(std::move(fs), id, vfs::node_type::file, vfs::node_mode::readable) {
    }
};

struct iterator_token : vfs::directory_iterator_token {
    uint32_t entry_pos;
    uint32_t entry_len = 0;
};

class directory : public vfs::directory {
public:
    uint32_t entry_count() const override {
        return num_entries();
    }

private:
    expected<vfs::node_ptr, errors> do_open(vfs::dir_entry_id_t id,
                                       vfs::node_mode mode) const override {
        return fs()->open(id.m_id, mode);
    }
public:
    expected<vfs::dir_entry_id_t, errors> lookup(std::string_view name) const override {
        return unexpected(errors{vfs::error_code::not_supported});
    }

    vfs::directory_iterator_token* iteration_begin() const override {
        if (entry_count() == 0) {
            return nullptr;
        }
        return new iterator_token{{}, 0};
    }

    vfs::entry_info iteration_entry_info(vfs::directory_iterator_token* token,
                                         span<uint8_t> name_buf) const override {
        auto tok = static_cast<iterator_token*>(token);
        auto res = get_entry_info(tok->entry_pos, name_buf);
        tok->entry_len = sizeof(node_id_t) + 1 + res.name_length;
        return res;
    }

    bool iteration_next(vfs::directory_iterator_token* token) const override {
        auto tok = static_cast<iterator_token*>(token);
        if (tok->entry_len == 0) {
            iteration_entry_info(token, tos::empty_span<uint8_t>());
        }
        tok->entry_pos += tok->entry_len;
        if (tok->entry_pos + sizeof(uint32_t) == f.file_body().size()) {
            delete tok;
            return true;
        }
        return false;
    }

    vfs::entry_info get_entry_info(int pos, span<uint8_t> name_buf) const {
        // First slice for total number of entries.
        // Second slice takes us to the specific entry
        auto entry_start = f.file_body().slice(sizeof(uint32_t)).slice(pos);

        vfs::entry_info res;
        memcpy(&res.node_id, entry_start.data(), sizeof res.node_id);

        uint8_t name_len;
        memcpy(&name_len, entry_start.slice(sizeof res.node_id).data(), 1);
        res.name_length = name_len;

        auto name_span =
            entry_start.slice(sizeof res.node_id).slice(1).slice(0, name_len);

        safe_span_copy(name_buf, name_span);

        return res;
    }

    uint32_t num_entries() const {
        auto start = f.file_body();
        uint32_t entries;
        memcpy(&entries, start.data(), 4);
        return entries;
    }

    directory(const vfs::fs_ptr& fs, node_id_t id)
        : vfs::directory(fs, id, vfs::node_type::directory, vfs::node_mode::readable)
        , f(fs, id) {
    }

private:
    file f;

    expected<void, errors> do_add_entry(std::string_view name,
                                        const node& node) override {
        return unexpected(errors{vfs::error_code::not_supported});
    }
    expected<void, errors> do_remove_entry(std::string_view name,
                                           const node& node) override {
        return unexpected(errors{vfs::error_code::not_supported});
    }
};

expected<vfs::node_ptr, errors>
tos::mappedromfs::filesystem::do_open(node_id_t id, vfs::node_mode mode) {
    auto size_span = m_data.slice(id, 4);
    uint32_t size;
    memcpy(&size, size_span.data(), 4);
    size >>= 31U;
    size &= 1;
    if (size == 0) {
        // This is a directory
        return make_intrusive<directory>(vfs::fs_ptr(this), id);
    } else {
        return make_intrusive<file>(vfs::fs_ptr(this), id);
    }
}
} // namespace

vfs::fs_ptr mount(span<const uint8_t> data) {
    return make_intrusive<filesystem>(data);
}
} // namespace tos::mappedromfs