#include <tos/gui/elements/image.hpp>

namespace tos::gui::elements {
void image::draw(const draw_context& ctx) {
    ctx.painter->draw_bitmap(m_image.color_type(),
                             m_image.get_data_raw(),
                             m_image.stride(),
                             gfx2::rectangle{{0, 0}, m_image.dimensions()},
                             ctx.bounds);
}
} // namespace tos::gui::elements