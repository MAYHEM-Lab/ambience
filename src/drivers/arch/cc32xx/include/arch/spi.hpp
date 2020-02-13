//
// Created by fatih on 11/6/19.
//

#pragma once

#include <cstdint>
#include <ti/drivers/SPI.h>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

namespace tos::cc32xx {
enum class spi_errors {};

class spi : public tracked_driver<spi, 2>, public self_pointing<spi> {
public:
    explicit spi(int index) : tracked_driver(index) {
        [[maybe_unused]] static auto _ = []{
          return SPI_init(), 0;
        }();
        SPI_Params params;
        SPI_Params_init(&params);
        params.bitRate = 4'000'000;
        params.transferMode = SPI_TransferMode::SPI_MODE_CALLBACK;
        params.transferCallbackFxn = [](SPI_Handle,
                                        SPI_Transaction *) {
            spi::get(0)->m_done.up_isr();
        };
        m_handle = SPI_open(index, &params);
    }

    expected<void, spi_errors> write(tos::span<const uint8_t> data);

    SPI_Handle native_handle() {
        return m_handle;
    }
private:
    semaphore m_done{0};
    SPI_Handle m_handle;
};
}

// impl

namespace tos::cc32xx {
inline expected<void, spi_errors> spi::write(tos::span<const uint8_t> data) {
    SPI_Transaction transaction{};
    transaction.count = data.size();
    transaction.txBuf = const_cast<uint8_t*>(data.data());
    while (!SPI_transfer(native_handle(), &transaction));
    m_done.down();
    return {};
}
}
