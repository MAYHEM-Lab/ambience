//
// Created by fatih on 10/14/19.
//

#pragma once

#include <tos/gfx/dimensions.hpp>
#include <common/gpio.hpp>
#include <tos/thread.hpp>
#include <tos/span.hpp>

namespace tos {
namespace gde021a1_constants {
constexpr uint8_t waveform_lookup_table[] = {
    0x82, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA,
    0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xAA, 0x00, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA,
    0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x15, 0x15, 0x15,
    0x15, 0x05, 0x05, 0x05, 0x05, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x41, 0x45, 0xF1, 0xFF, 0x5F, 0x55, 0x01, 0x00, 0x00, 0x00};

constexpr auto EPD_REG_0 = 0x00;  /* Status Read */
constexpr auto EPD_REG_1 = 0x01;  /* Driver Output Control */
constexpr auto EPD_REG_3 = 0x03;  /* Gate driving voltage control */
constexpr auto EPD_REG_4 = 0x04;  /* Source driving coltage control */
constexpr auto EPD_REG_7 = 0x07;  /* Display Control */
constexpr auto EPD_REG_11 = 0x0B; /* Gate and Sorce non overlap period COntrol */
constexpr auto EPD_REG_15 = 0x0F; /* Gate scan start */
constexpr auto EPD_REG_16 = 0x10; /* Deep Sleep mode setting */
constexpr auto EPD_REG_17 = 0x11; /* Data Entry Mode Setting */
constexpr auto EPD_REG_18 = 0x12; /* SWRESET */
constexpr auto EPD_REG_26 =
    0x1A; /* Temperature Sensor Control (Write to Temp Register) */
constexpr auto EPD_REG_27 =
    0x1B; /* Temperature Sensor Control(Read from Temp Register) */
constexpr auto EPD_REG_28 =
    0x1C; /* Temperature Sensor Control(Write Command  to Temp sensor) */
constexpr auto EPD_REG_29 = 0x1D;  /* Temperature Sensor Control(Load temperature register
                                     with temperature sensor \  reading) */
constexpr auto EPD_REG_32 = 0x20;  /* Master activation */
constexpr auto EPD_REG_33 = 0x21;  /* Display update */
constexpr auto EPD_REG_34 = 0x22;  /* Display update control 2 */
constexpr auto EPD_REG_36 = 0x24;  /* write RAM */
constexpr auto EPD_REG_37 = 0x25;  /* Read RAM */
constexpr auto EPD_REG_40 = 0x28;  /* VCOM sense */
constexpr auto EPD_REG_41 = 0x29;  /* VCOM Sense duration */
constexpr auto EPD_REG_42 = 0x2A;  /* VCOM OTP program */
constexpr auto EPD_REG_44 = 0x2C;  /* Write VCOMregister */
constexpr auto EPD_REG_45 = 0x2D;  /* Read OTP registers */
constexpr auto EPD_REG_48 = 0x30;  /* Program WS OTP */
constexpr auto EPD_REG_50 = 0x32;  /* Write LUT register */
constexpr auto EPD_REG_51 = 0x33;  /* Read LUT register */
constexpr auto EPD_REG_54 = 0x36;  /* Program OTP selection */
constexpr auto EPD_REG_55 = 0x37;  /* Proceed OTP selection */
constexpr auto EPD_REG_58 = 0x3A;  /* Set dummy line pulse period */
constexpr auto EPD_REG_59 = 0x3B;  /* Set Gate line width */
constexpr auto EPD_REG_60 = 0x3C;  /* Select Border waveform */
constexpr auto EPD_REG_68 = 0x44;  /* Set RAM X - Address Start / End Position */
constexpr auto EPD_REG_69 = 0x45;  /* Set RAM Y - Address Start / End Position */
constexpr auto EPD_REG_78 = 0x4E;  /* Set RAM X Address Counter */
constexpr auto EPD_REG_79 = 0x4F;  /* Set RAM Y Address Counter */
constexpr auto EPD_REG_240 = 0xF0; /* Booster Set Internal Feedback Selection */
constexpr auto EPD_REG_255 = 0xFF; /* NOP */
} // namespace gde021a1_constants

template<class SpiT>
class gde021a1 {
public:
    using GpioT = typename std::remove_pointer_t<SpiT>::gpio_type;
    using PinT = typename GpioT::pin_type;

    static constexpr auto HEIGHT = 72;
    static constexpr auto WIDTH = 172;

    template<class DelayT>
    explicit gde021a1(SpiT spi,
                      PinT chip_select,
                      PinT data_command,
                      PinT reset,
                      PinT busy,
                      DelayT&& delay)
        : m_spi{std::move(spi)}
        , m_cs{chip_select}
        , m_dc{data_command}
        , m_reset{reset}
        , m_busy{busy} {
        m_gpio.set_pin_mode(m_reset, tos::pin_mode::out);
        m_gpio.write(m_reset, tos::digital::high);
        m_gpio.set_pin_mode(m_busy, tos::pin_mode::in);
        hard_reset(delay);
        using namespace std::chrono_literals;
        delay(5ms);
        init();
    }

    void refresh_display() {
        using namespace gde021a1_constants;
        writeReg(EPD_REG_34, 0xC4);
        gde021a1WriteRegArray(EPD_REG_32, nullptr, 0);
        wait_while_busy();
    }

    void set_display_window(const tos::gfx::point& pos,
                            const tos::gfx::dimensions& dims) {
        using namespace gde021a1_constants;

        /* Set Y position and the height */
        select_register(EPD_REG_68);
        write_data(pos.y);
        write_data(dims.height / 4 - 1);
        /* Set X position and the width */
        select_register(EPD_REG_69);
        write_data(pos.x);
        write_data(dims.width - 1);
        /* Set the height counter */
        select_register(EPD_REG_78);
        write_data(pos.y);
        /* Set the width counter */
        select_register(EPD_REG_79);
        write_data(pos.x);
    }

    void draw_framebuffer(const tos::gfx::dimensions& dimensions,
                          tos::span<const uint8_t> framebuf_data) {
        using namespace gde021a1_constants;

        /* Prepare the register to write data on the RAM */
        select_register(EPD_REG_36);

        for (int i = 0; i < dimensions.height * dimensions.width / 8; i++) {
            uint8_t pixels_4 = framebuf_data[i];
            if (pixels_4 == 0) {
                write_data(0x00);
                write_data(0x00);
                continue;
            }

            for (int iter = 0; iter < 2; iter++) {
                uint8_t data = 0;

                for (int j = 0; j < 3; ++j) {
                    if (pixels_4 & 0x80U) {
                        data |= 0b11U;
                    }
                    data <<= 2U;
                    pixels_4 <<= 1U;
                }
                if (pixels_4 & 0x80U) {
                    data |= 0b11U;
                }
                pixels_4 <<= 1U;

                write_data(data);
            }
        }
    }

private:
    void wait_while_busy() {
        while (m_gpio.read(m_busy)) {
            tos::this_thread::yield();
        }
    }

    void gde021a1WriteRegArray(uint8_t cmd, const uint8_t* data, uint8_t numDataBytes) {
        m_gpio->write(m_dc, tos::digital::low);
        m_gpio->write(m_cs, tos::digital::low);
        m_spi->write({&cmd, 1});
        m_gpio->write(m_dc, tos::digital::high);
        m_spi->write({data, numDataBytes});
        m_gpio->write(m_cs, tos::digital::high);
    }

    void writeReg(uint8_t cmd, uint8_t data) {
        gde021a1WriteRegArray(cmd, &data, 1);
    }

    template<class DelayT>
    void hard_reset(DelayT&& delay) {
        using namespace std::chrono_literals;
        m_gpio->write(m_reset, tos::digital::low);
        delay(20ms);
        m_gpio->write(m_reset, tos::digital::high);
        delay(20ms);
    }

    void init() {
        using namespace gde021a1_constants;
        writeReg(EPD_REG_16, 0x00); // Deep sleep mode disable
        writeReg(EPD_REG_17, 0x03); // Data Entry Mode Setting

        // This sets the start and end addresses in the Y direction
        // RAM X address start = 0x00
        // RAM X address end = 0x11 (17 * 4 px/addr = 72 pixels)
        uint8_t temp[2] = {0x00, 0x11};
        gde021a1WriteRegArray(EPD_REG_68, temp, 2);

        // This sets the start and end addresses in the Y direction
        // RAM Y address start = 0x00
        // RAM Y address end = 0xAB
        temp[0] = 0x00;
        temp[1] = 0xAB;
        gde021a1WriteRegArray(EPD_REG_69, temp, 2);

        writeReg(EPD_REG_78, 0x00);  // Set RAM X Address counter
        writeReg(EPD_REG_79, 0x00);  // Set RAM Y Address counter
        writeReg(EPD_REG_240, 0x1F); // Booster Set Internal Feedback Selection
        writeReg(EPD_REG_33, 0x03);  // Disable RAM bypass and set GS transition
        // to GSA = GS0 and GSB = GS3
        writeReg(EPD_REG_44, 0xA0); // Write VCOM Register
        writeReg(EPD_REG_60, 0x64); // Border waveform
        gde021a1WriteRegArray(EPD_REG_50,
                              waveform_lookup_table,
                              std::size(waveform_lookup_table)); // Write LUT
    }

    void select_register(uint8_t c) {
        tos::pull_low_guard pull_low_dc(m_gpio, m_dc);
        tos::pull_low_guard pull_low_cs(m_gpio, m_cs);

        m_spi->write(tos::monospan(c));
    }

    void write_data(uint8_t d) {
        tos::pull_low_guard pull_low_cs(m_gpio, m_cs);
        m_spi->write(tos::monospan(d));
    }

    SpiT m_spi;
    GpioT m_gpio;
    PinT m_cs, m_dc, m_reset, m_busy;
};
} // namespace tos