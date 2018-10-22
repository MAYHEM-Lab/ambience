//
// Created by fatih on 6/14/18.
//

#pragma once

#include <tos/delay.hpp>
#include <tos/span.hpp>

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001 // Register select bit

#include <common/i2c.hpp>

namespace tos
{
    template <class I2cT>
    class lcd
    {
    public:
        lcd(I2cT& i2c, twi_addr_t lcd_addr, uint8_t cols, uint8_t rows)
                : m_i2c{i2c}, m_addr{lcd_addr}, m_rows{rows}, m_cols{cols} {
            m_backlight = LCD_BACKLIGHT;
        }

        template <class AlarmT>
        void begin(AlarmT& alarm);

        void command(uint8_t);
        void clear();

        void display();
        void no_display();

        void home();

        void backlight();
        void no_backlight();

        void set_cursor(uint8_t col, uint8_t row);
        void print(span<const char> buf);

        void write(char c);
    private:

        void send(uint8_t, uint8_t);
        void write4bits(uint8_t);
        void expanderWrite(uint8_t);
        void pulseEnable(uint8_t);

        I2cT& m_i2c;
        twi_addr_t m_addr;
        uint8_t m_rows, m_cols;
        uint8_t m_backlight;
        uint8_t m_display_mode;
        uint8_t m_display_func;
        uint8_t m_display_ctrl;
    };
}

// IMPL

namespace tos
{
    template<class I2cT>
    template <class AlarmT>
    void lcd<I2cT>::begin(AlarmT& alarm) {
        auto displ_func = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

        if (m_rows > 1)
        {
            displ_func |= LCD_2LINE;
        }

        alarm.sleep_for({ 50 });

        expanderWrite(m_backlight);
        alarm.sleep_for({ 1000 });

        write4bits(0x03 << 4);
        alarm.sleep_for({ 5 });

        write4bits(0x03 << 4);
        alarm.sleep_for({ 5 });

        write4bits(0x03 << 4);
        tos::delay_us(microseconds{150});

        // finally, set to 4-bit interface
        write4bits(0x02 << 4);
        // set # lines, font size, etc.
        command(LCD_FUNCTIONSET | m_display_func);

        // turn the display on with no cursor or blinking default
        m_display_ctrl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
        display();

        // clear it off
        clear();

        // Initialize to default text direction (for roman languages)
        m_display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

        // set the entry mode
        command(LCD_ENTRYMODESET | m_display_mode);

        home();
    }

    template<class I2cT>
    void lcd<I2cT>::command(uint8_t value) {
        send(value, 0);
    }

    template<class I2cT>
    void lcd<I2cT>::send(uint8_t value, uint8_t mode) {
        uint8_t highnib = value & 0xF0;
        uint8_t lownib=(value<<4)&0xF0;
        write4bits((highnib)|mode);
        write4bits((lownib)|mode);
    }

    template<class I2cT>
    void lcd<I2cT>::write4bits(uint8_t value) {
        expanderWrite(value);
        pulseEnable(value);
    }

    template<class I2cT>
    void lcd<I2cT>::expanderWrite(uint8_t _data) {
        auto data = uint8_t((int)(_data) | m_backlight);
        auto res = m_i2c.transmit(m_addr, { (char*)&data, 1 });
        if (res != twi_tx_res::ok)
        {
            //TODO: err
        }
    }

    template<class I2cT>
    void lcd<I2cT>::pulseEnable(uint8_t _data) {
        expanderWrite(_data | En);	// En high
        tos::delay_us(microseconds{1});

        expanderWrite(_data & ~En);	// En low
        tos::delay_us(microseconds{50});
    }

    template<class I2cT>
    void lcd<I2cT>::clear() {
        command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
        tos::delay_ms(milliseconds{2}); // should probably be a timer sleep
    }

    template<class I2cT>
    void lcd<I2cT>::display() {
        m_display_ctrl |= LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | m_display_ctrl);
    }

    template<class I2cT>
    void lcd<I2cT>::no_display() {
        m_display_ctrl &= ~LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | m_display_ctrl);
    }

    template<class I2cT>
    void lcd<I2cT>::home() {
        command(LCD_RETURNHOME);  // set cursor position to zero
        tos::delay_ms(milliseconds{2});
    }

    template<class I2cT>
    void lcd<I2cT>::set_cursor(uint8_t col, uint8_t row) {
        int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
        command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
    }

    template<class I2cT>
    void lcd<I2cT>::write(char c) {
        send(c, Rs);
    }

    template<class I2cT>
    void lcd<I2cT>::print(span<const char> buf) {
        for (auto c : buf)
        {
            if (c == 0) break;
            write(c);
        }
    }

    template<class I2cT>
    void lcd<I2cT>::backlight() {
        m_backlight = LCD_BACKLIGHT;
        expanderWrite(0);
    }

    template<class I2cT>
    void lcd<I2cT>::no_backlight() {
        m_backlight = LCD_NOBACKLIGHT;
        expanderWrite(0);
    }
}
