//
// Created by fatih on 8/31/19.
//

#pragma once

#include <common/ssd1306/constants.hpp>
#include <cstdint>
#include <tos/span.hpp>

namespace tos {
template<class I2CT>
struct ssd1306 {
public:
    ssd1306(I2CT i2c, tos::twi_addr_t i2c_addr, uint16_t cols, uint16_t rows);

    void dim(bool dim);

    void set_pixel(int row, int col, bool color);

    int height() const { return m_h; }
    int width() const { return m_w; }

    void display();

private:
    static constexpr inline auto vccstate = detail::SSD1306_SWITCHCAPVCC;

    void initialize();

    void single_command(uint8_t c);

    void command_list(const uint8_t* c, int n, uint8_t first = 0);

    struct framebuffer {
        framebuffer(uint16_t w, uint16_t h)
            : m_buf(w * h, 0) {
        }

        std::vector<uint8_t> m_buf;
    };

    I2CT m_i2c;
    tos::twi_addr_t m_addr;
    uint16_t m_w, m_h;
    framebuffer m_fb;
};

template<class I2CT>
ssd1306<I2CT>::ssd1306(I2CT i2c, tos::twi_addr_t i2c_addr, uint16_t cols, uint16_t rows)
    : m_i2c(i2c)
    , m_addr{i2c_addr}
    , m_w{cols}
    , m_h{rows}
    , m_fb(m_w, (m_h + 7) / 8) {
    initialize();
    display();
}

template<class I2CT>
void ssd1306<I2CT>::dim(bool dim) {
    using namespace detail;
    single_command(SSD1306_SETCONTRAST);
    single_command(dim ? 0 : (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
}

template<class I2CT>
void ssd1306<I2CT>::set_pixel(int row, int col, bool color) {
    if (color) {
        m_fb.m_buf[col + (row / 8) * m_w] |= (1 << (row % 8));
    } else {
        m_fb.m_buf[col + (row / 8) * m_w] &= ~(1 << (row % 8));
    }
}

template<class I2CT>
void ssd1306<I2CT>::display() {
    using namespace detail;
    static const uint8_t dlist1[] = {SSD1306_PAGEADDR,
                                     0,    // Page start address
                                     0xFF, // Page end (not really, but works here)
                                     SSD1306_COLUMNADDR,
                                     0}; // Column start address
    command_list(dlist1, sizeof(dlist1));
    single_command(m_w - 1); // Column end address

    command_list(m_fb.m_buf.data(), m_fb.m_buf.size(), 0x40);
}

template<class I2CT>
void ssd1306<I2CT>::initialize() {
    using namespace detail;
    static const uint8_t init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                                    SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                                    0x80,                  // the suggested ratio 0x80
                                    SSD1306_SETMULTIPLEX}; // 0xA8
    command_list(init1, sizeof(init1));
    single_command(m_h - 1);

    static const uint8_t init2[] = {SSD1306_SETDISPLAYOFFSET,   // 0xD3
                                    0x0,                        // no offset
                                    SSD1306_SETSTARTLINE | 0x0, // line #0
                                    SSD1306_CHARGEPUMP};        // 0x8D
    command_list(init2, sizeof(init2));

    single_command((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    static const uint8_t init3[] = {SSD1306_MEMORYMODE, // 0x20
                                    0x00,               // 0x0 act like ks0108
                                    SSD1306_SEGREMAP | 0x1,
                                    SSD1306_COMSCANDEC};
    command_list(init3, sizeof(init3));

    if ((m_w == 128) && (m_h == 32)) {
        static const uint8_t init4a[] = {SSD1306_SETCOMPINS, // 0xDA
                                         0x02,
                                         SSD1306_SETCONTRAST, // 0x81
                                         0x8F};
        command_list(init4a, sizeof(init4a));
    } else if ((m_w == 128) && (m_h == 64)) {
        static const uint8_t init4b[] = {SSD1306_SETCOMPINS, // 0xDA
                                         0x12,
                                         SSD1306_SETCONTRAST}; // 0x81
        command_list(init4b, sizeof(init4b));
        single_command((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
    } else if ((m_w == 96) && (m_h == 16)) {
        static const uint8_t init4c[] = {SSD1306_SETCOMPINS,   // 0xDA
                                         0x2,                  // ada x12
                                         SSD1306_SETCONTRAST}; // 0x81
        command_list(init4c, sizeof(init4c));
        single_command((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF);
    }

    single_command(SSD1306_SETPRECHARGE); // 0xd9
    single_command((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    static const uint8_t init5[] = {SSD1306_SETVCOMDETECT, // 0xDB
                                    0x40,
                                    SSD1306_DISPLAYALLON_RESUME, // 0xA4
                                    SSD1306_NORMALDISPLAY,       // 0xA6
                                    SSD1306_DEACTIVATE_SCROLL,
                                    SSD1306_DISPLAYON}; // Main screen turn on
    command_list(init5, sizeof(init5));
}

template<class I2CT>
void ssd1306<I2CT>::single_command(uint8_t c) {
    std::array<char, 2> buf = {0x00, static_cast<char>(c)};
    m_i2c->transmit(m_addr, buf);
}

template<class I2CT>
void ssd1306<I2CT>::command_list(const uint8_t* c, int n, uint8_t first) {
    std::array<char, 16> buf;
    buf[0] = first;

    while (n > 0) {
        auto len = std::min<int>(std::size(buf) - 1, n);
        std::copy(c, c + len, buf.begin() + 1);
        c += len;
        n -= len;

        m_i2c->transmit(m_addr, tos::span<char>(buf).slice(0, len + 1));

        tos::this_thread::yield();
    }
}
} // namespace tos
