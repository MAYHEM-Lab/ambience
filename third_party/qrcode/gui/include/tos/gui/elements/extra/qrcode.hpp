#pragma once

#include <cstdint>
#include <string_view>
#include <tos/gui/elements.hpp>
#include <vector>

namespace tos::qr {
enum class error_correction
{
    low,
    medium,
    quartile,
    high
};
}

namespace tos::gui::elements {
class qrcode_element : public tos::gui::element {
public:
    explicit qrcode_element(std::string_view data,
                            qr::error_correction ecc = qr::error_correction::medium);

    tos::gui::view_limits limits(const tos::gui::draw_context&) const {
        return {{m_size, m_size}, {tos::gui::full_extent, tos::gui::full_extent}};
    }

    void draw(const tos::gui::draw_context& ctx);

private:
    int m_size;
    std::vector<uint8_t> m_buffer;
};
} // namespace tos::gui::elements