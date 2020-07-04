#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::decorators {
template<class Base>
struct padding {
    Base base;
    int left, right, top, bottom;

    auto size() {
        tos::gfx2::size base_size = base.size();
        return tos::gfx2::size{base_size.width() + left + right,
                               base_size.height() + top + bottom};
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        tos::gfx2::size base_size = at.dims();
        auto actual_size = tos::gfx2::size{base_size.width() - left - right,
                                           base_size.height() - top - bottom};
        base.draw(painter,
                  {{at.corner().x() + left, at.corner().y() + top}, actual_size});
    }
};

template<class Base>
padding(Base&& base, int left, int right, int top, int bottom) -> padding<Base>;

template<class Base>
struct bordered {
    Base base;

    auto size() {
        return base.size();
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        base.draw(painter, at);

        tos::services::style s(tos::gfx2::binary_color{false});
        painter.set_style(s);
        painter.draw_rect(at, 0, false);
    }
};

template<class Base>
bordered(Base&& b) -> bordered<Base>;

template<class Base>
struct round_bordered {
    int radius;
    Base base;

    auto size() {
        return base.size();
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        base.draw(painter, at);

        tos::services::style s(tos::gfx2::binary_color{false});
        painter.set_style(s);
        painter.draw_rect(at, radius, false);
    }
};

template<class Base>
round_bordered(int radius, Base&& b) -> round_bordered<Base>;

template<class Base>
struct fixed_size {
    Base base;
    tos::gfx2::size sz;

    auto size() {
        return sz;
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        base.draw(painter, {at.corner(), sz});
    }
};

template<class Base>
fixed_size(Base&& b, const tos::gfx2::size&) -> fixed_size<Base>;

template<class Base>
struct align_center_middle {
    Base base;

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        auto base_size = base.size();
        auto x_diff = (at.dims().width() - base_size.width()) / 2;
        auto y_diff = (at.dims().height() - base_size.height()) / 2;
        base.draw(painter,
                  {{at.corner().x() + x_diff, at.corner().y() + y_diff}, base_size});
    }
};

template<class Base>
align_center_middle(Base&& b) -> align_center_middle<Base>;
} // namespace tos::gui::decorators