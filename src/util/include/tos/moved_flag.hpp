//
// Created by fatih on 9/18/19.
//

#pragma once

namespace tos {
struct moved_flag {
    moved_flag() = default;
    moved_flag(moved_flag&& rhs) noexcept {
        rhs.m_moved = true;
    }

    explicit operator bool() const {
        return m_moved;
    }

    moved_flag& operator=(moved_flag&& rhs) = delete;

private:
    bool m_moved = false;
};
} // namespace tos