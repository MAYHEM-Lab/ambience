#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
class label : public element {
public:
    explicit label(std::string str)
        : m_str{std::move(str)} {
    }

    tos::gfx2::size size() const {
        return {m_str.size() * 8, 8};
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at);

private:
    std::string m_str;
};
}