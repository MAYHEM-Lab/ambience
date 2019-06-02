//
// Created by fatih on 6/1/19.
//

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <common/gpio.hpp>
#include <tos/ft.hpp>

namespace tos
{
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE 0xFF

static constexpr uint8_t LUTDefault_full[] =
{
    WRITE_LUT_REGISTER,  // command
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

static constexpr uint8_t LUTDefault_part[] =
{
    WRITE_LUT_REGISTER,  // command
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

template <class SpiT>
class epd {
public:
    using gpio_type = typename std::remove_pointer_t<SpiT>::gpio_type;
    using PinT = typename gpio_type::pin_type;

    static const uint16_t WIDTH = 128;
    static const uint16_t HEIGHT = 296;

    uint16_t width = 128;
    uint16_t height = 296;

    explicit epd(SpiT spi, PinT cs, PinT dc, PinT reset, PinT busy)
        : m_spi{std::move(spi)}
        , m_cs{cs}
        , m_dc{dc}
        , m_reset{reset}
        , m_busy{busy}
    {
        m_g.set_pin_mode(m_dc, tos::pin_mode::out);
        m_g.write(m_dc, tos::digital::high);

        m_g.set_pin_mode(m_reset, tos::pin_mode::out);
        m_g.write(m_reset, tos::digital::high);

        m_g.set_pin_mode(m_busy, tos::pin_mode::in);
    }

    template <class DelayT>
    void hard_reset(DelayT&& delay)
    {
        using namespace std::chrono_literals;
        m_g.write(m_reset, tos::digital::low);
        delay(200ms);
        m_g.write(m_reset, tos::digital::high);
        delay(200ms);
    }

    template <class DelayT>
    void initialize(DelayT&& delay)
    {
        hard_reset(delay);
        _writeCommand(DRIVER_OUTPUT_CONTROL); // Panel configuration, Gate selection
        _writeData((HEIGHT - 1) % 256);
        _writeData((HEIGHT - 1) / 256);
        _writeData(0x00);
        _writeCommand(BOOSTER_SOFT_START_CONTROL); // softstart
        _writeData(0xd7);
        _writeData(0xd6);
        _writeData(0x9d);
        _writeCommand(WRITE_VCOM_REGISTER); // VCOM setting
        _writeData(0xa8);    // * different
        _writeCommand(SET_DUMMY_LINE_PERIOD); // DummyLine
        _writeData(0x1a);    // 4 dummy line per gate
        _writeCommand(SET_GATE_TIME); // Gatetime
        _writeData(0x08);    // 2us per line
        _writeCommand(BORDER_WAVEFORM_CONTROL);
        _writeData(0x03);
        _writeCommand(DATA_ENTRY_MODE_SETTING);
        _writeData(0x03); // X increment; Y increment
        _writeCommandDataPGM(LUTDefault_full, sizeof(LUTDefault_full));
        //_setPartialRamArea(0, 0, WIDTH, HEIGHT);
    }

    void SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
        SendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
        /* x point must be the multiple of 8 or the last 3 bits will be ignored */
        SendData((x_start >> 3) & 0xFF);
        SendData((x_end >> 3) & 0xFF);
        SendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
        SendData(y_start & 0xFF);
        SendData((y_start >> 8) & 0xFF);
        SendData(y_end & 0xFF);
        SendData((y_end >> 8) & 0xFF);
    }

    void SetMemoryPointer(int x, int y) {
        SendCommand(SET_RAM_X_ADDRESS_COUNTER);
        /* x point must be the multiple of 8 or the last 3 bits will be ignored */
        SendData((x >> 3) & 0xFF);
        SendCommand(SET_RAM_Y_ADDRESS_COUNTER);
        SendData(y & 0xFF);
        SendData((y >> 8) & 0xFF);
        WaitUntilIdle();
    }

    void ClearFrameMemory(unsigned char color) {
        SetMemoryArea(0, 0, this->width - 1, this->height - 1);
        SetMemoryPointer(0, 0);
        SendCommand(WRITE_RAM);
        /* send the color data */
        for (int i = 0; i < this->width / 8 * this->height; i++) {
            SendData(color);
        }
    }

    void DisplayFrame() {
        SendCommand(DISPLAY_UPDATE_CONTROL_2);
        SendData(0xC4);
        SendCommand(MASTER_ACTIVATION);
        SendCommand(TERMINATE_FRAME_READ_WRITE);
        WaitUntilIdle();
    }

    void SetFrameMemory(
        const unsigned char* image_buffer,
        int x,
        int y,
        int image_width,
        int image_height
    ) {
        int x_end;
        int y_end;

        if (
            image_buffer == NULL ||
            x < 0 || image_width < 0 ||
            y < 0 || image_height < 0
            ) {
            return;
        }
        /* x point must be the multiple of 8 or the last 3 bits will be ignored */
        x &= 0xF8;
        image_width &= 0xF8;
        if (x + image_width >= this->width) {
            x_end = this->width - 1;
        } else {
            x_end = x + image_width - 1;
        }
        if (y + image_height >= this->height) {
            y_end = this->height - 1;
        } else {
            y_end = y + image_height - 1;
        }
        SetMemoryArea(x, y, x_end, y_end);
        SetMemoryPointer(x, y);
        SendCommand(WRITE_RAM);
        /* send the image data */
        for (int j = 0; j < y_end - y + 1; j++) {
            for (int i = 0; i < (x_end - x + 1) / 8; i++) {
                SendData(image_buffer[i + j * (image_width / 8)]);
            }
        }
    }

    void WaitUntilIdle()
    {
        while (m_g.read(m_busy)) {
            tos::this_thread::yield();
        }
    }

private:

    void SendCommand(unsigned char command) {
        _writeCommand(command);
    }

    void SendData(unsigned char data) {
        _writeData(data);
    }

    void _writeCommand(uint8_t c)
    {
        m_g.write(m_dc, tos::digital::low);
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(c);
        m_g.write(m_cs, tos::digital::high);
        m_g.write(m_dc, tos::digital::high);
    }

    void _writeData(uint8_t d)
    {
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(d);
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeData(const uint8_t* data, uint16_t n) {
        m_g.write(m_cs, tos::digital::low);
        m_spi->write({data, n});
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeCommandData(const uint8_t* pCommandData, uint8_t datalen)
    {
        m_g.write(m_dc, tos::digital::low);
        m_g.write(m_cs, tos::digital::low);
        m_spi->write(*pCommandData++);
        datalen--;
        m_g.write(m_dc, tos::digital::high);
        m_spi->write({pCommandData, datalen});
        m_g.write(m_cs, tos::digital::high);
    }

    void _writeCommandDataPGM(const uint8_t* pCommandData, uint8_t datalen)
    {
        std::vector<uint8_t> buf(pCommandData, pCommandData + datalen);
        _writeCommandData(buf.data(), buf.size());
    }

    void _writeDataPGM(const uint8_t* data, uint16_t n)
    {
        std::vector<uint8_t> buf(data, data + n);
        _writeData(buf.data(), buf.size());
    }

    SpiT m_spi;
    gpio_type m_g;
    PinT m_cs, m_dc, m_reset, m_busy;
};
}