#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

namespace tos::x86_64 {
enum class text_vga_color : uint8_t
{
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    magenta = 5,
    brown = 6,
    light_grey = 7,
    dark_grey = 8,
    light_blue = 9,
    light_green = 10,
    light_cyan = 11,
    light_red = 12,
    light_magenta = 13,
    light_brown = 14,
    white = 15,
};

class text_vga : public self_pointing<text_vga> {
public:
    static constexpr size_t width = 80;
    static constexpr size_t heigth = 25;

    text_vga() {
        set_color(text_vga_color::white, text_vga_color::black);
    }

    void clear() {
        for (size_t y = 0; y < heigth; y++) {
            for (size_t x = 0; x < width; x++) {
                auto index = y * width + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
        }
        terminal_row = 0;
        terminal_column = 0;
    }

    void set_color(text_vga_color fg, text_vga_color bg) {
        terminal_color = vga_entry_color(fg, bg);
    }

    void write_line(int num, std::string_view line) {
        auto row = get_row(num);
        size_t i;
        for (i = 0; i < std::min(row.size(), line.size()); i++) {
            row[i] = vga_entry(line[i], terminal_color);
        }
        for (; i < row.size(); ++i) {
            row[i] = vga_entry(' ', terminal_color);
        }
    }

    void write(std::string_view str) {
        for (auto c : str) {
            put_char(c);
        }
    }

    int write(span<const uint8_t> data) {
        for (auto c : data) {
            put_char(c);
        }
        return data.size();
    }

private:
    static constexpr uint8_t vga_entry_color(text_vga_color fg, text_vga_color bg) {
        return uint8_t(fg) | uint8_t(bg) << 4;
    }

    static constexpr uint16_t vga_entry(uint8_t uc, uint8_t color) {
        return (uint16_t)uc | (uint16_t)color << 8;
    }

    void put_char(char c) {
        if (c == 0) {
            return;
        }

        if (c == '\n') {
            if (++terminal_row == heigth) {
                terminal_row = 0;
            }
            auto r = get_row(terminal_row);
//            std::fill(r.begin(), r.end(), vga_entry(' ', terminal_color));
            return;
        }

        if (c == '\r') {
            terminal_column = 0;
            return;
        }

        write_at(c, terminal_column, terminal_row);

        if (++terminal_column == width) {
            terminal_column = 0;
            if (++terminal_row == heigth) {
                terminal_row = 0;
            }
            auto r = get_row(terminal_row);
            std::fill(r.begin(), r.end(), vga_entry(' ', terminal_color));
        }
    }

    void write_at(char c, int x, int y) {
        const size_t index = y * width + x;
        terminal_buffer[index] = vga_entry(c, terminal_color);
    }

    span<uint16_t> get_row(int row) {
        const size_t index = row * width;
        return span<uint16_t>{terminal_buffer + index, width};
    }

    uint16_t terminal_row = 0;
    uint16_t terminal_column = 0;
    uint8_t terminal_color;
    uint16_t* terminal_buffer = reinterpret_cast<uint16_t*>(0xB8000);
};
} // namespace tos::x86_64
