#include <tos/build.hpp>
#include <tos/debug/log.hpp>
#include <tos/vfs/directory.hpp>
#include <tos/vfs/file.hpp>
#include <tos/vfs/filesystem.hpp>

namespace tos::testfs {
namespace {
using vfs::error_code;
using vfs::errors;
using vfs::fs_ptr;
using vfs::node_id_t;
using vfs::node_ptr;

struct iterator_token : vfs::directory_iterator_token {
    int idx;
};

class directory : public vfs::directory {
    uint32_t entry_count() const override {
        return 1;
    }

    expected<vfs::dir_entry_id_t, errors> lookup(std::string_view name) const override {
        if (name == "hello") {
            return vfs::dir_entry_id_t{1};
        }

        if (name == "commit") {
            return vfs::dir_entry_id_t{2};
        }

        return unexpected(errors{error_code::not_found});
    }

    expected<void, errors> do_add_entry(std::string_view name,
                                        const node& node) override {
        return unexpected(errors{error_code::not_supported});
    }

    expected<void, errors> do_remove_entry(std::string_view name,
                                           const node& node) override {
        return unexpected(errors{error_code::not_supported});
    }

    expected<node_ptr, errors> do_open(vfs::dir_entry_id_t id,
                                       vfs::node_mode mode) const override {
        return fs()->open(id.m_id, mode);
    }

public:
    vfs::directory_iterator_token* iteration_begin() const override {
        return new iterator_token{{}, 0};
    }
    vfs::entry_info iteration_entry_info(vfs::directory_iterator_token* token,
                                         span<uint8_t> name_buf) const override {
        switch (static_cast<iterator_token*>(token)->idx) {
        case 0:
            std::copy_n("hello", std::min<int>(5, name_buf.size()), name_buf.begin());
            return {5, vfs::dir_entry_id_t{1}};
        case 1:
            std::copy_n("commit", std::min<int>(6, name_buf.size()), name_buf.begin());
            return {6, vfs::dir_entry_id_t{2}};
        }
        TOS_UNREACHABLE();
    }
    bool iteration_next(vfs::directory_iterator_token* token) const override {
        if (static_cast<iterator_token*>(token)->idx == 1) {
            delete static_cast<iterator_token*>(token);
            return true;
        }
        static_cast<iterator_token*>(token)->idx++;
        return false;
    }

public:
    directory(fs_ptr fs, node_id_t id, vfs::node_type type, vfs::node_mode mode)
        : vfs::directory(std::move(fs), id, type, mode) {
    }
};

class file final : public vfs::file {
    static constexpr char contents[] = "hello world";

public:
    vfs::file_size_t size() const override {
        if (id() == 1) {
            return std::size(contents) - 1;
        }
        if (id() == 2) {
            return tos::build::commit_hash().size();
        }
        TOS_UNREACHABLE();
    }

private:
    expected<span<uint8_t>, errors> do_read(vfs::file_offset_t offset,
                                            span<uint8_t> data) override {
        if (id() == 1) {
            auto len = std::min<int>(size(), data.size());
            std::copy_n(std::begin(contents), len, data.begin());
            return data.slice(0, len);
        }
        if (id() == 2) {
            auto len = std::min<int>(size(), data.size());
            std::copy_n(std::begin(tos::build::commit_hash()), len, data.begin());
            return data.slice(0, len);
        }
        TOS_UNREACHABLE();
    }

    expected<size_t, errors> do_write(vfs::file_offset_t offset,
                                      span<const uint8_t> data) override {
        return unexpected(errors{error_code::not_supported});
    }

public:
    file(fs_ptr fs, node_id_t id, vfs::node_type type, vfs::node_mode mode)
        : vfs::file(std::move(fs), id, type, mode) {
    }
};

class filesystem : public vfs::filesystem {
    void collect_node(vfs::node* obj) override {
        delete obj;
    }

    vfs::node_id_t root_node_id() override {
        return 0;
    }

    void do_sync() override {
    }

    expected<node_ptr, vfs::errors> do_open(vfs::node_id_t id,
                                            vfs::node_mode mode) override {
        if (id == 0) {
            return make_intrusive<directory>(
                fs_ptr(this), id, vfs::node_type::directory, mode);
        }
        return make_intrusive<file>(fs_ptr(this), id, vfs::node_type::file, mode);
    }

public:
    filesystem()
        : vfs::filesystem(true) {
    }
};
} // namespace

fs_ptr open() {
    return make_intrusive<filesystem>();
}
} // namespace tos::testfs