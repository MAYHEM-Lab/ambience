#include <arch/flash.hpp>
#include <storage_generated.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/io/packet_transport.hpp>
#include <tos/io/serial_backend.hpp>

using bs = tos::bsp::board_spec;

template<class FlashT>
class flash_storage : public tos::services::block_storage {
public:
    explicit flash_storage(FlashT flash)
        : m_flash{std::move(flash)} {
    }

    int32_t get_block_size() override {
        return m_flash->sector_size_bytes();
    }

    int32_t get_block_count() override {
        return m_flash->number_of_sectors();
    }

    bool erase(const int32_t& block_idx) override {
        [[maybe_unused]] auto res = m_flash->erase(block_idx);
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

    tos::io::serial_backend transport(&com);
    auto log_channel = tos::io::make_channel(transport, 4);

    auto flash = bs::flash::open();
    flash_storage flash_service(&flash);
    auto runner = lidl::make_procedure_runner<tos::services::block_storage>();

    tos::launch(tos::alloc_stack, [&] {
        tos::io::poll_packets(transport,
                              tos::cancellation_token::system(),
                              [&](int stream, tos::intrusive_ptr<tos::io::packet>&& p) {
                                  if (stream == 4) {
                                      log_channel->receive(std::move(p));
                                      return;
                                  }

                                  std::array<uint8_t, 256> resp_buf;
                                  lidl::message_builder build(resp_buf);

                                  switch (stream) {
                                  case 5:
                                      runner(flash_service, as_span(*p), build);
                                      break;
                                  default:
                                      return;
                                  }

                                  auto response =
                                      build.get_buffer().slice(0, build.size());
                                  transport.send(stream, response);
                              });
    });

    using generated_remote_logger =
        tos::services::remote_logger<tos::io::packet_transport>;
    generated_remote_logger log(log_channel);
    tos::debug::lidl_sink sink(log);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::log);
    tos::debug::set_default_log(&logger);

    LOG("Hello world");

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, storage_main);
}