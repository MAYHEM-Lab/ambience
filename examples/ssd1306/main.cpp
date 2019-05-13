//
// Created by fatih on 12/6/18.
//

#include <common/lcd.hpp>
#include <tos/ft.hpp>
#include <arch/drivers.hpp>

#include <tos/version.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

namespace tos
{
    void delay_ms(std::chrono::milliseconds ms) {
        delay_us(ms);
    }

    void delay_us(std::chrono::microseconds us)
    {
        uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
        for (volatile int i = 0; i < end; ++i)
        {
            __asm__ __volatile__ ("nop");
        }
    }
} // namespace tos
#define SSD1306_MEMORYMODE          0x20 ///< See datasheet
#define SSD1306_COLUMNADDR          0x21 ///< See datasheet
#define SSD1306_PAGEADDR            0x22 ///< See datasheet
#define SSD1306_SETCONTRAST         0x81 ///< See datasheet
#define SSD1306_CHARGEPUMP          0x8D ///< See datasheet
#define SSD1306_SEGREMAP            0xA0 ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON        0xA5 ///< Not currently used
#define SSD1306_NORMALDISPLAY       0xA6 ///< See datasheet
#define SSD1306_INVERTDISPLAY       0xA7 ///< See datasheet
#define SSD1306_SETMULTIPLEX        0xA8 ///< See datasheet
#define SSD1306_DISPLAYOFF          0xAE ///< See datasheet
#define SSD1306_DISPLAYON           0xAF ///< See datasheet
#define SSD1306_COMSCANINC          0xC0 ///< Not currently used
#define SSD1306_COMSCANDEC          0xC8 ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET    0xD3 ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5 ///< See datasheet
#define SSD1306_SETPRECHARGE        0xD9 ///< See datasheet
#define SSD1306_SETCOMPINS          0xDA ///< See datasheet
#define SSD1306_SETVCOMDETECT       0xDB ///< See datasheet

#define SSD1306_SETLOWCOLUMN        0x00 ///< Not currently used
#define SSD1306_SETHIGHCOLUMN       0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE        0x40 ///< See datasheet

#define SSD1306_EXTERNALVCC         0x01 ///< External display voltage source
#define SSD1306_SWITCHCAPVCC        0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26 ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27 ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL                    0x2E ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 ///< Set scroll range

struct framebuffer
{
    framebuffer(uint16_t w, uint16_t h) : m_buf(w * h, 0) {
    }

    std::vector<uint8_t> m_buf;
};

struct ssd1306
{
public:
    //0x3D

    ssd1306(tos::stm32::twim& i2c, tos::twi_addr_t a, uint16_t cols, uint16_t rows)
        : m_addr{a}, m_i2c(i2c), m_w{cols}, m_h{rows}, m_fb(m_w, (m_h + 7) / 8) {
        init();
        display();
    }

    void dim(bool dim)
    {
        ssd1306_command1(SSD1306_SETCONTRAST);
        ssd1306_command1(dim ? 0 : (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
    }

    void set_pixel(int row, int col, bool color)
    {
        if (color)
        {
            m_fb.m_buf[col + (row/8) * m_w] |= (1 << (row & 7));
        }
        else
        {
            m_fb.m_buf[col + (row/8) * m_w] &= ~(1 << (row & 7));
        }
    }

    void display()
    {
        static const uint8_t dlist1[] = {
                SSD1306_PAGEADDR,
                0,                         // Page start address
                0xFF,                      // Page end (not really, but works here)
                SSD1306_COLUMNADDR,
                0 };                       // Column start address
        ssd1306_commandList(dlist1, sizeof(dlist1));
        ssd1306_command1(m_w - 1); // Column end address

        tos::this_thread::yield();

        ssd1306_commandList(m_fb.m_buf.data(), m_fb.m_buf.size(), 0x40);

        tos::this_thread::yield();
    }
private:

    static constexpr inline auto vccstate = SSD1306_SWITCHCAPVCC;
    void init()
    {
        static const uint8_t init1[] = {
                SSD1306_DISPLAYOFF,                   // 0xAE
                SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
                0x80,                                 // the suggested ratio 0x80
                SSD1306_SETMULTIPLEX };               // 0xA8
        ssd1306_commandList(init1, sizeof(init1));
        ssd1306_command1(m_h - 1);

        static const uint8_t init2[] = {
                SSD1306_SETDISPLAYOFFSET,             // 0xD3
                0x0,                                  // no offset
                SSD1306_SETSTARTLINE | 0x0,           // line #0
                SSD1306_CHARGEPUMP };                 // 0x8D
        ssd1306_commandList(init2, sizeof(init2));

        ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

        static const uint8_t init3[] = {
                SSD1306_MEMORYMODE,                   // 0x20
                0x00,                                 // 0x0 act like ks0108
                SSD1306_SEGREMAP | 0x1,
                SSD1306_COMSCANDEC };
        ssd1306_commandList(init3, sizeof(init3));

        if((m_w == 128) && (m_h == 32)) {
            static const uint8_t init4a[] = {
                    SSD1306_SETCOMPINS,                 // 0xDA
                    0x02,
                    SSD1306_SETCONTRAST,                // 0x81
                    0x8F };
            ssd1306_commandList(init4a, sizeof(init4a));
        } else if((m_w == 128) && (m_h == 64)) {
            static const uint8_t init4b[] = {
                    SSD1306_SETCOMPINS,                 // 0xDA
                    0x12,
                    SSD1306_SETCONTRAST };              // 0x81
            ssd1306_commandList(init4b, sizeof(init4b));
            ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF);
        } else if((m_w == 96) && (m_h == 16)) {
            static const uint8_t init4c[] = {
                    SSD1306_SETCOMPINS,                 // 0xDA
                    0x2,    // ada x12
                    SSD1306_SETCONTRAST };              // 0x81
            ssd1306_commandList(init4c, sizeof(init4c));
            ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF);
        }

        ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
        ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
        static const uint8_t init5[] = {
                SSD1306_SETVCOMDETECT,               // 0xDB
                0x40,
                SSD1306_DISPLAYALLON_RESUME,         // 0xA4
                SSD1306_NORMALDISPLAY,               // 0xA6
                SSD1306_DEACTIVATE_SCROLL,
                SSD1306_DISPLAYON };                 // Main screen turn on
        ssd1306_commandList(init5, sizeof(init5));
    }

    void ssd1306_command1(uint8_t c)
    {
        char buf[] = { 0x00, c };
        m_i2c->transmit(m_addr, buf);
    }

    void ssd1306_commandList(const uint8_t *c, int n, uint8_t first = 0)
    {
        char buf[16];
        buf[0] = first;

        while (n > 0)
        {
            auto len = std::min<int>(std::size(buf) - 1, n);
            std::copy(c, c + len, buf + 1);
            c += len;
            n -= len;

            m_i2c->transmit(m_addr, tos::spanify<char>(buf).slice(0, len + 1));
        }
    }


    uint16_t m_w, m_h;
    framebuffer m_fb;
    tos::twi_addr_t m_addr;
    tos::stm32::twim& m_i2c;
};

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

void lcd_main()
{
    using namespace tos::tos_literals;
    using namespace tos; using namespace tos::stm32;

    auto g = tos::open(tos::devs::gpio);

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;
    twim t { 24_pin, 25_pin };

    ssd1306 l{t, {0x3C}, 128, 64};
    l.dim(false);

    auto tmr = open(devs::timer<2>);
    auto alarm = open(devs::alarm, tmr);

    bool go = true;
    uint32_t x = 0;
    while (true)
    {
        using namespace std::chrono_literals;

        //tos::println(usart, "tick", x);

        for (int i = 0; i < 128; ++i)
        {
            l.set_pixel(x % 64, i, go);
        }
        l.display();
        if (x % 64 == 63){
            go ^= true;
            alarm->sleep_for(5s);
        }

        usart_setup(g);
        auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

        tos::println(usart, "tick", int(x));
        ++x;

        std::array<char, 1> b;
        usart->read(b, alarm, 10ms);
    }
}

void tos_main()
{
    tos::launch(tos::stack_size_t{512}, lcd_main);
}