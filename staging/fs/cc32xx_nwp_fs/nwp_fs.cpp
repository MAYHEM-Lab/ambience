/**
 * This file implements the vfs API on the Texas Instruments CC3x20, CC3x35 network
 * processor (NWP) from hell.
 *
 * The network processor exposes a filesystem which you can put 240 files in and you have
 * to use it for any kind of persistent data. You see, TI doesn't allow you to write to
 * the 1M flash you have on these things. If you write to it, the bootloader will refuse
 * to boot your system.
 *
 * You also have to use the filesystem if you'd like to use the TLS functionality of the
 * NWP. For instance, to set a Root CA, you first write the certificate in der format to
 * a file in the file system, and set the root ca to the name of that file. Amazing!
 *
 * Note that I said name, and not path. That's right! The FS is not hierarchical! Even if
 * you put forward slashes in your filenames, they are all in the same _directory_.
 *
 * Although the vfs does not impose hierarchies, a flat file system is pretty unexpected
 * and would probably break a bunch of programs. Therefore, this implementation imitates
 * directory structures by interpreting the paths.
 */

#include "nwp_fs.hpp"
#include <ti/drivers/net/wifi/fs.h>
#include <tos/debug/log.hpp>
#include <tos/string.hpp>
#include <tos/vfs/directory.hpp>
#include <tos/vfs/file.hpp>

namespace tos::cc32xx {
namespace {
using vfs::errors;
using vfs::node_id_t;
using vfs::node_ptr;

struct iter_state : vfs::directory_iterator_token {
    int32_t index = -1;

    struct slGetfileList_t {
        SlFileAttributes_t attribute;
        char fileName[SL_FS_MAX_FILE_NAME_LENGTH];
    } file;
};

class file final : public vfs::file {
public:
    vfs::file_size_t size() const override {
        return sl_FsGetInfo();
    }

private:
    expected<span<uint8_t>, errors> do_read(vfs::file_offset_t offset,
                                            span<uint8_t> data) override {
        auto res = sl_FsRead(id(), offset, data.data(), data.size());
        if (res < 0) {
            // error
        }
        return data.slice(res);
    }

    expected<size_t, errors> do_write(vfs::file_offset_t offset,
                                      span<const uint8_t> data) override {
        auto res =
            sl_FsWrite(id(), offset, const_cast<uint8_t*>(data.data()), data.size());
        if (res < 0) {
            // error
        }
        return res;
    }

private:
};

class root_directory final : public vfs::directory {
public:
    uint32_t entry_count() const override {
        _i32 Status;
        SlFsControlGetStorageInfoResponse_t GetStorageInfoResponse;
        Status = sl_FsCtl((SlFsCtl_e)SL_FS_CTL_GET_STORAGE_INFO,
                          0,
                          nullptr,
                          nullptr,
                          0,
                          (_u8*)&GetStorageInfoResponse,
                          sizeof(SlFsControlGetStorageInfoResponse_t),
                          nullptr);
        if (Status < 0) {
            return 0;
        }

        return GetStorageInfoResponse.FilesUsage.ActualNumOfUserFiles +
               GetStorageInfoResponse.FilesUsage.ActualNumOfSysFiles;
    }

    expected<node_id_t, errors> lookup(std::string_view name) const override {
        return unexpected(errors{vfs::error_code::not_supported});
    }

    vfs::directory_iterator_token* iteration_begin() const override {
        auto state = new iter_state;
        iteration_next(state);
        return state;
    }

    vfs::entry_info iteration_entry_info(vfs::directory_iterator_token* token,
                                         span<uint8_t> name_buf) const override {
        auto state = static_cast<iter_state*>(token);

        auto full_len = strlen(state->file.fileName);
        vfs::entry_info res;
        res.node_id = state->index;
        res.name_length = full_len - m_prefix.size();

        auto name_span = tos::span<const char>(state->file.fileName)
                             .slice(0, full_len)
                             .slice(m_prefix.size());
        safe_span_copy(name_buf, name_span);

        return res;
    }

    bool iteration_next(vfs::directory_iterator_token* token) const override {
        auto state = static_cast<iter_state*>(token);

        auto res = sl_FsGetFileList(
            &state->index,
            1,
            (_u8)(SL_FS_MAX_FILE_NAME_LENGTH + sizeof(SlFileAttributes_t)),
            (unsigned char*)&state->file,
            SL_FS_GET_FILE_ATTRIBUTES);

        if (res < 0) {
            LOG_ERROR("GetFileList failed", res);
            delete state;
            return true;
        }

        if (res == 0) {
            delete state;
            return true;
        }

        if (!starts_with(std::string_view(state->file.fileName), m_prefix)) {
            return iteration_next(token);
        }

        return false;
    }

    std::string_view m_prefix = "/";

private:
    expected<void, errors> do_add_entry(std::string_view name,
                                        const node& node) override {
        return unexpected(errors{vfs::error_code::not_supported});
    }

    expected<void, errors> do_remove_entry(std::string_view name,
                                           const node& node) override {
        return unexpected(errors{vfs::error_code::not_supported});
    }

public:
    root_directory(vfs::fs_ptr fs, node_id_t id)
        : vfs::directory(
              std::move(fs), id, vfs::node_type::directory, vfs::node_mode::read_write) {
    }
};

class filesystem : public vfs::filesystem {
public:
    void collect_node(vfs::node* obj) override {
    }

    node_id_t root_node_id() override {
        return 0xFFFF;
    }

private:
    void do_sync() override {
    }

    expected<node_ptr, errors> do_open(node_id_t id, vfs::node_mode mode) override {
        if (id == 0xFFFF) {
            return make_intrusive<root_directory>(vfs::fs_ptr(this), 0xFFFF);
        }
        return unexpected(errors{vfs::error_code::not_supported});
    }

public:
    filesystem()
        : vfs::filesystem(false) {
    }
};
} // namespace

expected<vfs::fs_ptr, vfs::errors> mount_nwp_fs() {
    return make_intrusive<filesystem>();
}
} // namespace tos::cc32xx