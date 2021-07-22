#pragma once

#include <tos/peripheral/vga_text.hpp>
#include <tos/self_pointing.hpp>
#include <deque>

struct scrollable : tos::self_pointing<scrollable> {
    void draw() {
        auto actual_top_line = m_top_line - m_lines_base;
        auto lines_to_draw = std::min<int>(m_vga.height, m_lines.size() - actual_top_line);
        for (int i = actual_top_line, j = 0; j < lines_to_draw; ++i, ++j) {
            m_vga.write_line(j, std::string_view(m_lines[i]));
        }
    }

    int write(tos::span<const uint8_t> data) {
        for (auto c : data) {
            if (c == '\n') {
                if (int(m_lines.size()) == m_max_lines) {
                    m_lines.pop_front();
                    m_lines_base += 1;
                }
                m_lines.emplace_back();
                m_lines.back().reserve(48);

                if (m_locked && m_lines.size() > m_vga.height) {
                    m_top_line += 1;
                }
                continue;
            } else if (c == '\r' || c == 0) {
                continue;
            }
            m_lines.back() += c;
        }
        draw();
        return data.size();
    }

    bool scroll_down() {
        if (m_top_line == int(m_lines.size()) - 1) {
            return false;
        }
        m_top_line += 1;
        if (m_top_line == int(m_lines.size()) - 1) {
            m_locked = true;
        }
        draw();
        return true;
    }

    bool scroll_up() {
        if (m_top_line == 0) {
            return false;
        }
        m_top_line -= 1;
        m_locked = false;
        draw();
        return true;
    }

    bool m_locked = true;
    int m_max_lines = 200;
    int m_top_line = 0;
    int m_lines_base = 0;
    std::deque<std::string> m_lines = std::deque<std::string>(1);
    tos::x86_64::text_vga m_vga{};
};