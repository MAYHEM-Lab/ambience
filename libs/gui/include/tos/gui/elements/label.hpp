#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
class label : public element {
public:
    explicit label(std::string str)
        : m_str{std::move(str)} {
    }

    view_limits limits(const draw_context& ctx) const {
        return { {m_str.size() * 8, 8}, {m_str.size() * 8, 8} };
    }

    void draw(const draw_context& ctx);

private:
    std::string m_str;
};
}