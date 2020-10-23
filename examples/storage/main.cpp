#include <arch/flash.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <storage_generated.hpp>

using bs = tos::bsp::board_spec;

template <class FlashT>
class flash_storage : public tos::services::block_storage {
public:
    int32_t get_block_size() override {
        return m_flash->sector_size_bytes();
    }

    int32_t get_block_count() override {
        return m_flash->number_of_sectors();
    }

    bool erase(const int32_t& block_idx) override {
        auto res = m_flash->erase(block_idx);
        return bool(res);
    }

    bool write(const int32_t& block_idx,
               tos::span<uint8_t> data,
               const int16_t& offset) override {
        auto res = m_flash->write(block_idx, data, offset);
        return bool(res);
    }

    tos::span<uint8_t> read(const int32_t& block_idx,
                            const int16_t& length,
                            const int16_t& offset,
                            lidl::message_builder& response_builder) override {
        auto buf = lidl::create_vector_sized<uint8_t>(response_builder, length);

        auto res = m_flash->read(block_idx, buf.span(), offset);

        if (!res) {
            LOG_WARN("Read failed!");
        }

        return buf.span();
    }

private:

    FlashT m_flash;
};

void storage_main() {
    auto com = bs::default_com::open();

    tos::debug::serial_sink sink{&com};
    tos::debug::detail::any_logger log_{&sink};
    log_.set_log_level(tos::debug::log_level::all);
    tos::debug::set_default_log(&log_);

    LOG("Hello");

    auto f = bs::flash::open();

    std::array<uint8_t, 8> buf;
    if (!f.read(100, buf, 0)) {
        LOG_ERROR("Read failed!");
    }

    LOG(buf);

    f.erase(100);
    uint8_t data[8] = {"hello"};
    if (!f.write(100, data, 0)) {
        LOG_ERROR("Write failed!");
    }

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, storage_main);
}