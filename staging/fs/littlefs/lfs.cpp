#include <tos/build.hpp>
#include <tos/debug/log.hpp>
#include <tos/vfs/directory.hpp>
#include <tos/vfs/file.hpp>
#include <tos/vfs/filesystem.hpp>

namespace tos::littlefs {
namespace {
using vfs::error_code;
using vfs::errors;
using vfs::fs_ptr;
using vfs::node_id_t;
using vfs::node_ptr;

class directory : public vfs::directory {
    uint32_t entry_count() const override {
        return 1;
    }

    expected<node_id_t, errors> lookup(std::string_view name) const override {
        if (name == "hello") {
            return 1;
        }

        if (name == "commit") {
            return 2;
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

public:
    directory(fs_ptr fs, node_id_t id, vfs::node_type type, vfs::node_mode mode)
        : vfs::directory(std::move(fs), id, type, mode) {
    }
};

class file final : public vfs::file {
    static constexpr char contents[] = "hello world";

public:
    vfs::file_size_t size() const override {
        if (native_handle() == 1) {
            return std::size(contents) - 1;
        }
        if (native_handle() == 2) {
            return tos::build::commit_hash().size();
        }
        TOS_UNREACHABLE();
    }

private:
    expected<span<uint8_t>, errors> do_read(vfs::file_offset_t offset,
                                            span<uint8_t> data) override {
        if (native_handle() == 1) {
            auto len = std::min<int>(size(), data.size());
            std::copy_n(std::begin(contents), len, data.begin());
            return data.slice(0, len);
        }
        if (native_handle() == 2) {
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
        LOG_TRACE("Collecting node", int(obj->native_handle()));
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
        : vfs::filesystem(false) {
    }
};
} // namespace

fs_ptr open() {
    return make_intrusive<filesystem>();
}
} // namespace tos::littlefs