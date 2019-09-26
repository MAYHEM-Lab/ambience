//
// Created by fatih on 5/21/19.
//

#include <arch/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/canvas.hpp>
#include <tos/gfx/text.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

auto delay = [](std::chrono::microseconds us) { HAL_Delay(us.count() / 1000); };

constexpr uint8_t WF_LUT[] = {
    0x82, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA,
    0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xAA, 0x00, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA,
    0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x15, 0x15, 0x15,
    0x15, 0x05, 0x05, 0x05, 0x05, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x41, 0x45, 0xF1, 0xFF, 0x5F, 0x55, 0x01, 0x00, 0x00, 0x00};

#define EPD_REG_0 0x00  /* Status Read */
#define EPD_REG_1 0x01  /* Driver Output Control */
#define EPD_REG_3 0x03  /* Gate driving voltage control */
#define EPD_REG_4 0x04  /* Source driving coltage control */
#define EPD_REG_7 0x07  /* Display Control */
#define EPD_REG_11 0x0B /* Gate and Sorce non overlap period COntrol */
#define EPD_REG_15 0x0F /* Gate scan start */
#define EPD_REG_16 0x10 /* Deep Sleep mode setting */
#define EPD_REG_17 0x11 /* Data Entry Mode Setting */
#define EPD_REG_18 0x12 /* SWRESET */
#define EPD_REG_26 0x1A /* Temperature Sensor Control (Write to Temp Register) */
#define EPD_REG_27 0x1B /* Temperature Sensor Control(Read from Temp Register) */
#define EPD_REG_28 0x1C /* Temperature Sensor Control(Write Command  to Temp sensor) */
#define EPD_REG_29                                                                       \
    0x1D /* Temperature Sensor Control(Load temperature register with temperature sensor \
            reading) */
#define EPD_REG_32 0x20  /* Master activation */
#define EPD_REG_33 0x21  /* Display update */
#define EPD_REG_34 0x22  /* Display update control 2 */
#define EPD_REG_36 0x24  /* write RAM */
#define EPD_REG_37 0x25  /* Read RAM */
#define EPD_REG_40 0x28  /* VCOM sense */
#define EPD_REG_41 0x29  /* VCOM Sense duration */
#define EPD_REG_42 0x2A  /* VCOM OTP program */
#define EPD_REG_44 0x2C  /* Write VCOMregister */
#define EPD_REG_45 0x2D  /* Read OTP registers */
#define EPD_REG_48 0x30  /* Program WS OTP */
#define EPD_REG_50 0x32  /* Write LUT register */
#define EPD_REG_51 0x33  /* Read LUT register */
#define EPD_REG_54 0x36  /* Program OTP selection */
#define EPD_REG_55 0x37  /* Proceed OTP selection */
#define EPD_REG_58 0x3A  /* Set dummy line pulse period */
#define EPD_REG_59 0x3B  /* Set Gate line width */
#define EPD_REG_60 0x3C  /* Select Border waveform */
#define EPD_REG_68 0x44  /* Set RAM X - Address Start / End Position */
#define EPD_REG_69 0x45  /* Set RAM Y - Address Start / End Position */
#define EPD_REG_78 0x4E  /* Set RAM X Address Counter */
#define EPD_REG_79 0x4F  /* Set RAM Y Address Counter */
#define EPD_REG_240 0xF0 /* Booster Set Internal Feedback Selection */
#define EPD_REG_255 0xFF /* NOP */


template<class SpiT>
class epd {
public:
    using PinT = tos::stm32::gpio::pin_type;

    static constexpr auto HEIGHT = 72;
    static constexpr auto WIDTH = 172;

    explicit epd(SpiT spi, PinT chip_select, PinT data_command, PinT reset, PinT busy)
        : m_spi{std::move(spi)}
        , m_cs{chip_select}
        , m_dc{data_command}
        , m_reset{reset}
        , m_busy{busy} {
        m_gpio.set_pin_mode(m_reset, tos::pin_mode::out);
        m_gpio.write(m_reset, tos::digital::high);
        m_gpio.set_pin_mode(m_busy, tos::pin_mode::in);
    }

    void gde021a1WriteRegArray(uint8_t cmd, uint8_t* data, uint8_t numDataBytes) {
        tos::stm32::gpio g;
        g.write(m_dc, tos::digital::low);
        g.write(m_cs, tos::digital::low);
        m_spi->write({&cmd, 1});
        g.write(m_dc, tos::digital::high);
        m_spi->write({data, numDataBytes});
        g.write(m_cs, tos::digital::high);
    }

    void writeReg(uint8_t cmd, uint8_t data) {
        gde021a1WriteRegArray(cmd, &data, 1);
    }

    void refreshDisplay() {
        writeReg(EPD_REG_34, 0xC4);
        gde021a1WriteRegArray(EPD_REG_32, nullptr, 0);
    }

    void gde021a1Init() {
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
        writeReg(EPD_REG_44, 0xA0);                              // Write VCOM Register
        writeReg(EPD_REG_60, 0x64);                              // Border waveform
        gde021a1WriteRegArray(EPD_REG_50, (uint8_t*)WF_LUT, 90); // Write LUT
    }

    void reset_controller() {
        using namespace std::chrono_literals;
        m_gpio.write(m_reset, tos::digital::low);
        delay(20ms);
        m_gpio.write(m_reset, tos::digital::high);
        delay(20ms);
    }

    void _waitWhileBusy() {
        while (m_gpio.read(m_busy)) {
            tos::this_thread::yield();
        }
    }

    void gde021a1Test() {

        uint8_t i = 0;
        uint8_t row = 0;
        uint8_t col = 0;
        gde021a1WriteRegArray(EPD_REG_36, NULL, 0);
        m_gpio.write(m_dc, tos::digital::high);
        for (col = 0; col < 172; col++) {
            for (row = 0; row < 18; row++) {
                // Checkerboard
                if (((col / 2) % 2) < 1) {
                    i = 0x0F;
                } else {
                    i = 0xF0;
                }

                // 4px bars
                if ((row % 2) > 0) {
                    i = 0xFF;
                } else {
                    i = 0x00;
                }
                // i = 0xFF;
                tos::pull_low_guard pull_low(m_gpio, m_cs);
                m_spi->write(tos::span<const uint8_t>{&i, 1});
            }
        }
        refreshDisplay();
    }

    void SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width) {
        /* Set Y position and the height */
        select_register(EPD_REG_68);
        write_data(Ypos);
        write_data(Height / 4 - 1);
        /* Set X position and the width */
        select_register(EPD_REG_69);
        write_data(Xpos);
        write_data(Width - 1);
        /* Set the height counter */
        select_register(EPD_REG_78);
        write_data(Ypos);
        /* Set the width counter */
        select_register(EPD_REG_79);
        write_data(Xpos);
    }

    void gde021a1_DrawImage(const uint16_t Xsize,
                            const uint16_t Ysize,
                            const uint8_t* pdata) {

        /* Prepare the register to write data on the RAM */
        select_register(EPD_REG_36);

        for (int i = 0; i < Ysize * Xsize / 8; i++) {
            uint8_t pixels_4 = pdata[i];
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
    tos::stm32::gpio m_gpio;
    tos::stm32::gpio::pin_type m_cs, m_dc, m_reset, m_busy;
};

void blink_task() {
    using namespace tos::tos_literals;

    auto chip_select = 15_pin;
    auto data_command = 27_pin;

    auto power_pin = 26_pin; // Active low

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(power_pin, tos::pin_mode::out);
    g.write(power_pin, tos::digital::low);

    g.set_pin_mode(chip_select, tos::pin_mode::out);
    g.write(chip_select, tos::digital::high);

    g.set_pin_mode(data_command, tos::pin_mode::out);
    g.write(data_command, tos::digital::high);

    auto timer = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, timer);

    tos::stm32::spi spi(tos::stm32::detail::spis[0], 19_pin, std::nullopt, 21_pin);

    g.set_pin_mode(5_pin, tos::pin_mode::out);

    epd<tos::stm32::spi*> display(&spi, chip_select, data_command, 18_pin, 8_pin);
    g.write(5_pin, tos::digital::high);
    display.reset_controller();
    g.write(5_pin, tos::digital::low);
    display.gde021a1Init();

    static tos::gfx::fixed_canvas<72, 172> frame_buffer;
    frame_buffer.fill(true);

    static constexpr auto font = tos::gfx::basic_font()
                                     .mirror_horizontal()
                                     .rotate_90_cw()
                                     .inverted();

    draw_text_line(frame_buffer,
                   font,
                   "tos",
                   tos::gfx::point{0, 0},
                   tos::gfx::text_direction::vertical);

    display.SetDisplayWindow(0, 0, 72, 172);
    display.gde021a1_DrawImage(72, 172, frame_buffer.data().data());
    display.refreshDisplay();

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::stack_size_t{2048}, blink_task);
}
