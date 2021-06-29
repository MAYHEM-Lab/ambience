#include <block_memory_generated.hpp>
#include <file_system_generated.hpp>
#include <lfs.h>
#include <log_generated.hpp>
#include <stdio.h>

namespace {
struct littlefs : tos::ae::services::filesystem::sync_server {
    littlefs(tos::ae::services::block_memory::sync_server* flash)
        : m_flash(flash) {
        m_conf.block_size = m_flash->get_block_size();
        m_conf.block_count = m_flash->get_block_count();
        m_conf.read_size = m_flash->get_block_size();
        m_conf.prog_size = m_flash->get_block_size();
        m_conf.cache_size = m_flash->get_block_size();
        m_conf.lookahead_size = 512;
        m_conf.block_cycles = 500;
        m_conf.context = this;
        m_conf.prog_buffer = new char[m_conf.cache_size];
        m_conf.read_buffer = new char[m_conf.cache_size];
        m_conf.lookahead_buffer = new char[m_conf.lookahead_size];

        m_conf.read = [](const struct lfs_config* c,
                         lfs_block_t block,
                         lfs_off_t off,
                         void* buffer,
                         lfs_size_t size) -> int {
            auto& self = *static_cast<littlefs*>(c->context);
            auto buf = tos::span(static_cast<uint8_t*>(buffer), size);
            lidl::message_builder builder{buf};
            auto res = self.m_flash->read(block, off, size, builder);
            return res.size() != buf.size();
        };

        m_conf.prog = [](const struct lfs_config* c,
                         lfs_block_t block,
                         lfs_off_t off,
                         const void* buffer,
                         lfs_size_t size) {
            auto& self = *static_cast<littlefs*>(c->context);
            auto buf = tos::span(
                const_cast<uint8_t*>(static_cast<const uint8_t*>(buffer)), size);
            auto res = self.m_flash->write(block, off, buf);
            if (!res) {
                return -1;
            }
            return 0;
        };

        m_conf.erase = [](const struct lfs_config* c, lfs_block_t block) {
            auto& self = *static_cast<littlefs*>(c->context);
            auto res = self.m_flash->erase(block);
            if (!res) {
                return -1;
            }
            return 0;
        };

        m_conf.sync = [](const struct lfs_config* c) { return 0; };

        printf("calling lfs_mount");
        auto res = lfs_mount(&m_lfs, &m_conf);
        printf("lfs_mount: %d", int(res));

        if (res != LFS_ERR_OK) {
            res = lfs_format(&m_lfs, &m_conf);
            printf("lfs_format: %d", int(res));

            res = lfs_mount(&m_lfs, &m_conf);
            printf("lfs_mount: %d", int(res));
        }
    }

    tos::ae::services::file_handle open(std::string_view path) override {
        auto file_ptr = new lfs_file;
        auto str = std::string(path);
        auto open_res =
            lfs_file_open(&m_lfs, file_ptr, str.c_str(), LFS_O_RDWR | LFS_O_CREAT);
        printf("open %s: %d, %p", str.c_str(), open_res, file_ptr);
        return {reinterpret_cast<uintptr_t>(file_ptr)};
    }

    tos::span<uint8_t> read_file(const tos::ae::services::file_handle& file,
                                 const uint32_t& at,
                                 const uint32_t& len,
                                 lidl::message_builder& response_builder) override {
        auto file_ptr = reinterpret_cast<lfs_file*>(file.priv());
        auto buffer = response_builder.allocate(len, 1);
        auto seek_res = lfs_file_seek(&m_lfs, file_ptr, at, LFS_SEEK_SET);
        printf("seek %p: %d", file_ptr, seek_res);
        auto read_res = lfs_file_read(&m_lfs, file_ptr, buffer, len);
        printf("read %p, %d, %d: %d", file_ptr, at, len, read_res);
        return tos::span<uint8_t>(buffer, read_res);
    }

    bool write_file(const tos::ae::services::file_handle& file,
                    const uint32_t& at,
                    tos::span<uint8_t> data) override {
        auto file_ptr = reinterpret_cast<lfs_file*>(file.priv());
        auto seek_res = lfs_file_seek(&m_lfs, file_ptr, at, LFS_SEEK_SET);
        printf("seek %p: %d", file_ptr, seek_res);
        auto res = lfs_file_write(&m_lfs, file_ptr, data.data(), data.size());
        printf("write %p, %d, %p, %d: %d", file_ptr, at, data.data(), data.size(), res);
        return res == data.size();
    }

    bool close_file(const tos::ae::services::file_handle& file) override {
        auto file_ptr = reinterpret_cast<lfs_file*>(file.priv());
        auto close_res = lfs_file_close(&m_lfs, file_ptr);
        return close_res >= 0;
    }

    lfs_t m_lfs{};
    lfs_config m_conf{};
    tos::ae::services::block_memory::sync_server* m_flash;
};
} // namespace

tos::ae::services::filesystem::sync_server*
make_littlefs_server(tos::ae::services::block_memory::sync_server* flash) {
    return new littlefs(flash);
}