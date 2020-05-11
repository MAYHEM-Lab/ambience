#pragma once

#include <memory>
#include <tos/gfx/dimensions.hpp>
#include <tos/gfx/painter.hpp>

namespace tos::x86 {
struct display_impl;
class display {
public:
    explicit display(const gfx::dimensions& dims);

    std::unique_ptr<gfx::painter> get_painter();

    ~display();
private:
    std::unique_ptr<display_impl> m_impl;
};
} // namespace tos::x86