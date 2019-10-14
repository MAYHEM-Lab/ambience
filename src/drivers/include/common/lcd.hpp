//
// Created by fatih on 6/14/18.
//

#pragma once

#include "driver_base.hpp"

#include <common/detail/lcd_constants.hpp>
#include <common/i2c.hpp>
#include <tos/delay.hpp>
#include <tos/span.hpp>

namespace tos {
/**
 * This class template implements a driver for the common character LCD screens
 * that have an I2C IO expander interface.
 * @tparam I2cT type of the I2C driver
 */
template<class I2cT>
class lcd : public self_pointing<lcd<I2cT>> {
public:
    lcd(I2cT i2c, twi_addr_t lcd_addr, uint8_t cols, uint8_t rows)
        : m_i2c{std::move(i2c)}
        , m_addr{lcd_addr}
        , m_rows{rows}
        , m_cols{cols} {
        m_backlight = LCD_BACKLIGHT;
    }

    template<class AlarmT>
    void begin(AlarmT& alarm);

    /**
     * Clears the display
     */
    void clear();

    /**
     * Turns on the display
     */
    void display();

    /**
     * Turns off the display
     */
    void no_display();

    /**
     * Places the cursor to the home position
     */
    void home();

    /**
     * Turns on the backlight of the display
     */
    void backlight();

    /**
     * Turns off the backlight of the display
     */
    void no_backlight();

    /**
     * Places the cursor to the given coordinates in
     * the screen
     */
    void set_cursor(uint8_t col, uint8_t row);

    /**
     * Writes the given array of characters to the screen
     * at the current cursor location.
     * @param buf buffer to write to the screen
     * @return number of bytes written
     */
    int write(span<const char> buf);

    void write(char c);

private:
    void command(uint8_t value);

    void send(uint8_t, uint8_t);
    void write4bits(uint8_t);
    void expanderWrite(uint8_t);
    void pulseEnable(uint8_t);

    I2cT m_i2c;
    twi_addr_t m_addr;
    uint8_t m_rows, m_cols;
    uint8_t m_backlight;
    uint8_t m_display_mode;
    uint8_t m_display_func;
    uint8_t m_display_ctrl;
};
} // namespace tos

// IMPL

namespace tos {
template<class I2cT>
template<class AlarmT>
void lcd<I2cT>::begin(AlarmT& alarm) {
    uint8_t displ_func = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

    if (m_rows > 1) {
        displ_func |= LCD_2LINE;
    }

    m_display_func = displ_func;

    using namespace std::chrono_literals;

    expanderWrite(m_backlight);
    alarm.sleep_for(200ms);

    write4bits(0x30);
    alarm.sleep_for(5ms);

    write4bits(0x30);
    alarm.sleep_for(5ms);

    write4bits(0x30);
    tos::delay_us(150us);

    // finally, set to 4-bit interface
    write4bits(0x20);
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
    uint8_t lownib = (value << 4) & 0xF0;
    write4bits(highnib | mode);
    write4bits(lownib | mode);
}

template<class I2cT>
void lcd<I2cT>::write4bits(uint8_t value) {
    expanderWrite(value);
    pulseEnable(value);
}

template<class I2cT>
void lcd<I2cT>::expanderWrite(uint8_t _data) {
    char data[] = {uint8_t(_data | m_backlight)};
    auto res = m_i2c->transmit(m_addr, data);
    if (res != twi_tx_res::ok) {
        // TODO: err
    }
}

template<class I2cT>
void lcd<I2cT>::pulseEnable(uint8_t _data) {
    using namespace std::chrono_literals;
    expanderWrite(_data | En); // En high
    tos::delay_us(1us);

    expanderWrite(_data & ~En); // En low
    tos::delay_us(50us);
}

template<class I2cT>
void lcd<I2cT>::clear() {
    using namespace std::chrono_literals;
    command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
    tos::delay_ms(2ms);        // should probably be a timer sleep
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
    using namespace std::chrono_literals;
    command(LCD_RETURNHOME); // set cursor position to zero
    tos::delay_ms(2ms);
}

template<class I2cT>
void lcd<I2cT>::set_cursor(uint8_t col, uint8_t row) {
    static constexpr int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

template<class I2cT>
void lcd<I2cT>::write(char c) {
    send(c, Rs);
}

template<class I2cT>
int lcd<I2cT>::write(span<const char> buf) {
    for (auto c : buf) {
        if (c == 0)
            break;
        write(c);
    }
    return buf.size();
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
} // namespace tos
