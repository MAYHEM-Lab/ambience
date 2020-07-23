#pragma once

#include <tos/gui/element.hpp>
#include <tos/gfx2/bitmap.hpp>

namespace tos::gui::elements {
class image : public element {
public:
    explicit image(const gfx2::bitmap& img) : m_image{img} {}

    view_limits limits(const draw_context&) const {
        return { {0, 0}, {full_extent, full_extent} };
    }
    
    void draw(const draw_context& ctx);

private:

    gfx2::bitmap m_image;
};
}