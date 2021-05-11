#include <qrcode.h>
#include <tos/gui/elements.hpp>
#include <tos/gui/elements/extra/qrcode.hpp>

namespace tos::gui::elements {
qrcode_element::qrcode_element(std::string_view data, qr::error_correction ecc) {
    QRCode qrcode;

    // Allocate a chunk of memory to store the QR code
    std::vector<uint8_t> qrcodeBytes(qrcode_getBufferSize(3));

    qrcode_initBytes(&qrcode,
                     qrcodeBytes.data(),
                     3,
                     static_cast<qrcode_error_correction>(ecc),
                     reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())),
                     data.size());

    m_buffer.resize(qrcode.size * qrcode.size);

    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            m_buffer[y * qrcode.size + x] = !qrcode_getModule(&qrcode, x, y);
        }
    }

    m_size = qrcode.size;
}

void qrcode_element::draw(const draw_context& ctx) {
    tos::gfx2::bitmap checkerboard(
        tos::gfx2::color::alternatives::binary, m_buffer, {m_size, m_size});

    using namespace tos::gui::elements;
    image(checkerboard).draw(ctx);
}
} // namespace tos::gui::elements