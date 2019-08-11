//
// Created by fatih on 7/21/19.
//

#pragma once

#include <tos/mutex.hpp>
#include "hal.h"
#include "stm32_bluenrg_ble.h"
#include "SPBTLE_RF.h"
#include "bluenrg_aci.h"
#include "bluenrg_gap.h"

#include <range/v3/view/zip.hpp>
#include <common/spi.hpp>

inline int spbtle_rf::spi_write(tos::span<const uint8_t> d1, tos::span<const uint8_t> d2) {
    int32_t result = 0;

    tos::lock_guard lock(m_spi_prot);
    Disable_SPI_IRQ();

    tos::pull_low_guard cs_guard{m_gpio, m_cs};
    unsigned char header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
    unsigned char header_slave[5] = {0xaa, 0x00, 0x00, 0x00, 0x00};


    /*for (auto& [s, h] :
            ranges::zip_view(header_slave, header_master))
    {
        s = m_spi->exchange8(h);
    }*/

    for (int i = 0; i < 5; ++i) {
        auto res = tos::spi::exchange(m_spi, header_master[i]);
        if (!res) return -1;
        header_slave[i] = force_get(res);
    }

    if (header_slave[0] == 0x02) {
        /* SPI is ready */
        if (header_slave[1] >= (d1.size() + d2.size())) {
            auto r1 = m_spi->write(d1);
            if (!r1) return -1;
            r1 = m_spi->write(d2);
            if (!r1) return -1;
        } else {
            /* Buffer is too small */
            result = -2;
        }
    } else {
        /* SPI is not ready */
        result = -1;
    }

    Enable_SPI_IRQ();

    return result;
}

inline int spbtle_rf::spi_read(tos::span<uint8_t> buf) {
    tos::lock_guard lock(m_spi_prot);

    tos::pull_low_guard cs_guard{m_gpio, m_cs};

    uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};
    uint8_t header_slave[5];

    for (int i = 0; i < 5; ++i) {
        auto res = tos::spi::exchange(m_spi, header_master[i]);
        if (!res) return -1;
        header_slave[i] = force_get(res);
    }

    uint16_t byte_count = 0;
    if (header_slave[0] == 0x02) {
        /* device is ready */
        byte_count = (uint16_t(header_slave[4]) << 8) | header_slave[3];

        if (byte_count > 0) {
            byte_count = std::min<uint16_t>(byte_count, buf.size());

            for (uint16_t len = 0; len < byte_count; len++) {
                auto res = tos::spi::exchange(m_spi, 0xff);
                if (!res) {

                }
                buf[len] = force_get(res);
            }
        }
    }

    // Add a small delay to give time to the BlueNRG to set the IRQ pin low
    // to avoid a useless SPI read at the end of the transaction
    for (int i = 0; i < 2; i++) asm("nop");

    return byte_count;
}

inline void spbtle_rf::bootloader() {
    Disable_SPI_IRQ();

    m_gpio.set_pin_mode(m_irq_pin, tos::pin_mode::out);
    m_gpio.write(m_irq_pin, tos::digital::high);

    BlueNRG_RST();
    Enable_SPI_IRQ();
}

inline tos::expected<tos::spbtle::fw_id, spbtle_errors> spbtle_rf::get_fw_id() const {
    tos::spbtle::fw_id build_number{};
    auto res = aci_hal_get_fw_build_number(&build_number.build_number);
    if (res)
    {
        return tos::unexpected(spbtle_errors::unknown);
    }
    return build_number;
}

inline void tos::spbtle::gap::set_device_name(std::string_view name) {
    auto ret = aci_gatt_update_char_value(
            service_handle,
            dev_name_char_handle,
            0,
            name.size(),
            (uint8_t *) name.data());
    if (ret) {
        tos::kern::fatal("Can't set name");
    }
    m_name_len = name.size();
}

inline tos::spbtle::gap::gap(tos::spbtle::gatt&, std::string_view name) {
    auto ret = aci_gap_init_IDB05A1(
            1,
            false,
            name.size(),
            &service_handle,
            &dev_name_char_handle,
            &appearance_char_handle);
    if (ret) {
        tos::kern::fatal("Can't init gap");
    }
    set_device_name(name);
}

inline tos::spbtle::gap::~gap() {
}

inline tos::spbtle::gap spbtle_rf::initialize_gap(tos::spbtle::gatt& g, std::string_view name) {
    return tos::spbtle::gap(g, name);
}

inline tos::expected<tos::spbtle::gatt, spbtle_errors>
spbtle_rf::initialize_gatt() {
    auto res = aci_gatt_init();
    if (res)
    {
        return tos::unexpected(spbtle_errors::unknown);
    }
    return tos::spbtle::gatt{};
}


inline tos::spbtle::advertising::advertising(
        std::chrono::milliseconds interval, std::string_view local_name) {
    constexpr auto CONN_INTERVAL_MIN_MS = 40;
    constexpr auto CONN_INTERVAL_MAX_MS = 150;
    uint8_t serviceUUIDList[] = {AD_TYPE_16_BIT_SERV_UUID, 0x34, 0x12};
    auto ret = aci_gap_set_discoverable(ADV_IND,
                                        (interval.count() * 1000) / 625,
                                        (interval.count() * 1000) / 625,
                                        STATIC_RANDOM_ADDR,
                                        NO_WHITE_LIST_USE,
                                        local_name.size(),
                                        local_name.data(),
                                        3,
                                        serviceUUIDList,
                                        (CONN_INTERVAL_MIN_MS * 1000) / 1250,
                                        (CONN_INTERVAL_MAX_MS * 1000) / 1250);
    if (ret != BLE_STATUS_SUCCESS)
    {
        tos::kern::fatal("Can't set discoverable");
    }
}

