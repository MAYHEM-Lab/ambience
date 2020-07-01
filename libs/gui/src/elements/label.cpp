#include <tos/gui/elements/label.hpp>

namespace tos::gui::elements {
void label::draw(services::painter& painter, const gfx2::rectangle& at) {
    painter.draw_text(m_str, at.corner());
}
}