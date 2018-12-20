//
// Created by fatih on 12/7/18.
//

#pragma once

namespace tos
{
    constexpr auto LCD_CLEARDISPLAY = 0x01;
    constexpr auto LCD_RETURNHOME = 0x02;
    constexpr auto LCD_ENTRYMODESET = 0x04;
    constexpr auto LCD_DISPLAYCONTROL = 0x08;
    constexpr auto LCD_CURSORSHIFT = 0x10;
    constexpr auto LCD_FUNCTIONSET = 0x20;
    constexpr auto LCD_SETCGRAMADDR = 0x40;
    constexpr auto LCD_SETDDRAMADDR = 0x80;

    // flags for display entry mode
    constexpr auto LCD_ENTRYRIGHT = 0x00;
    constexpr auto LCD_ENTRYLEFT = 0x02;
    constexpr auto LCD_ENTRYSHIFTINCREMENT = 0x01;
    constexpr auto LCD_ENTRYSHIFTDECREMENT = 0x00;

    // flags for display on/off control
    constexpr auto LCD_DISPLAYON = 0x04;
    constexpr auto LCD_DISPLAYOFF = 0x00;
    constexpr auto LCD_CURSORON = 0x02;
    constexpr auto LCD_CURSOROFF = 0x00;
    constexpr auto LCD_BLINKON = 0x01;
    constexpr auto LCD_BLINKOFF = 0x00;

    // flags for display/cursor shift
    constexpr auto LCD_DISPLAYMOVE = 0x08;
    constexpr auto LCD_CURSORMOVE = 0x00;
    constexpr auto LCD_MOVERIGHT = 0x04;
    constexpr auto LCD_MOVELEFT = 0x00;

    // flags for function set
    constexpr auto LCD_8BITMODE = 0x10;
    constexpr auto LCD_4BITMODE = 0x00;
    constexpr auto LCD_2LINE = 0x08;
    constexpr auto LCD_1LINE = 0x00;
    constexpr auto LCD_5x10DOTS = 0x04;
    constexpr auto LCD_5x8DOTS = 0x00;

    // flags for backlight control
    constexpr auto LCD_BACKLIGHT = 0x08;
    constexpr auto LCD_NOBACKLIGHT = 0x00;

    constexpr auto En = 0b00000100;  // Enable bit
    constexpr auto Rw = 0b00000010;  // Read/Write bit
    constexpr auto Rs = 0b00000001; // Register select bit
}