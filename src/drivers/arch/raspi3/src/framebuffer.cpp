//
// Created by fatih on 3/11/20.
//

#include <arch/framebuffer.hpp>
#include <arch/messagebox.hpp>

namespace tos::raspi3 {
namespace {
enum class fb_tags
{
    allocate_buffer = 0x00040001,
    release_buffer = 0x00048001,
    screen_size = 0x00048003,
    virtual_screen_size = 0x00048004,
    set_bitdepth = 0x00048005
};
}

raspi3::framebuffer::framebuffer(gfx::dimensions dims) {
    property_channel props;

    {
        property_channel_tags_builder builder;
        auto buf = builder
                       .add(static_cast<uint32_t>(fb_tags::screen_size),
                            {dims.width, dims.height})
                       .add(static_cast<uint32_t>(fb_tags::virtual_screen_size),
                            {dims.width, dims.height})
                       .add(static_cast<uint32_t>(fb_tags::set_bitdepth), {24})
                       .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }

        m_physical.width = buf[5];
        m_physical.height = buf[6];
    }
    {
        property_channel_tags_builder builder;
        auto buf = builder.add(static_cast<uint32_t>(fb_tags::allocate_buffer), {4096, 0}).end();
        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }

        m_buffer = span<uint8_t>(reinterpret_cast<uint8_t*>(buf[5] & 0x3FFFFFFF), buf[6]);
    }
}
} // namespace tos::raspi3