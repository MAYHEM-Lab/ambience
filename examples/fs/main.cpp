#include <arch/drivers.hpp>
#include <lfs.h>
#include <tos/expected.hpp>
#include <tos/span.hpp>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

enum class flash_errors
{

};

using tos::span;
using tos::expected;

class in_memory_flash : public tos::non_copy_movable {
public:
    using sector_id_t = uint16_t;
    in_memory_flash(
        tos::span<uint8_t> buffer,
        size_t block_size) : m_buffer{buffer}, m_block_size{block_size} {}

    size_t sector_size_bytes() const {
        return m_block_size;
    }

    size_t sector_count() const {
        return m_buffer.size() / sector_size_bytes();
    }

    expected<void, flash_errors> erase(sector_id_t sector_id) {
        auto sec = sector_span(sector_id);
        std::fill(sec.begin(), sec.end(), 0xFF);
        return {};
    }

    expected<void, flash_errors>
    write(sector_id_t sector_id, span<const uint8_t> data, uint16_t offset) {
        auto sec = sector_span(sector_id).slice(offset);
        std::copy(data.begin(), data.end(), sec.begin());
        return {};
    }

    expected<span<uint8_t>, flash_errors>
    read(sector_id_t sector_id, span<uint8_t> data, uint16_t offset) {
        auto sec = sector_span(sector_id).slice(offset);
        std::copy(sec.begin(), sec.end(), data.begin());
        return data;
    }

private:

    span<uint8_t> sector_span(sector_id_t sector) {
        return m_buffer.slice(m_block_size * sector, m_block_size);
    }

    tos::span<uint8_t> m_buffer;
    size_t m_block_size;
};

namespace tos::lfs {
template<class FlashT>
class lfs : public self_pointing<lfs<FlashT>> {
public:
    explicit lfs(FlashT, bool format = false);

    ~lfs();
    lfs_t m_lfs{};
private:
    lfs_config m_conf{};
    FlashT m_flash;
};
} // namespace tos::lfs

namespace tos::lfs {
template<class FlashT>
lfs<FlashT>::lfs(FlashT flash, bool format)
    : m_flash{std::move(flash)} {
    m_conf.block_size = m_flash->sector_size_bytes();
    m_conf.block_count = m_flash->sector_count();
    m_conf.read_size = 16;
    m_conf.prog_size = 16;
    m_conf.cache_size = 512;
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
                   lfs_size_t size) {
        auto& self = *static_cast<lfs*>(c->context);
        auto buf = span(static_cast<uint8_t*>(buffer), size);
        auto res = self.m_flash->read(block, buf, off);
        if (!res) {
            return -1;
        }
        return 0;
    };

    m_conf.prog = [](const struct lfs_config* c,
                   lfs_block_t block,
                   lfs_off_t off,
                   const void* buffer,
                   lfs_size_t size) {
        auto& self = *static_cast<lfs*>(c->context);
        auto buf = span(static_cast<const uint8_t*>(buffer), size);
        auto res = self.m_flash->write(block, buf, off);
        if (!res) {
            return -1;
        }
        return 0;
    };

    m_conf.erase = [](const struct lfs_config* c, lfs_block_t block) {
        auto& self = *static_cast<lfs*>(c->context);
        auto res = self.m_flash->erase(block);
        if (!res) {
            return -1;
        }
        return 0;
    };

    m_conf.sync = [](const struct lfs_config* c) {
        return 0;
    };

    if (format) {
        lfs_format(&m_lfs, &m_conf);
    }

    auto res = lfs_mount(&m_lfs, &m_conf);
    if (res) {
        tos::debug::panic("lfs_mount failed");
    }
}

template <class FlashT>
lfs<FlashT>::~lfs() {
    lfs_unmount(&m_lfs);
}
} // namespace tos::lfs


void fs_task() {
    auto fd = open("/tmp/flash", O_RDWR);
    ftruncate(fd, 64*2048);
    auto ptr = static_cast<uint8_t*>(mmap(nullptr, 64 * 2048, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    in_memory_flash flash({ptr,64 * 2048}, 2048);
    tos::lfs::lfs fs(&flash);

    lfs_file_t file{};
    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&fs.m_lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&fs.m_lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&fs.m_lfs, &file);
    lfs_file_write(&fs.m_lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&fs.m_lfs, &file);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}

void tos_main() {
    tos::launch(tos::alloc_stack, fs_task);
}