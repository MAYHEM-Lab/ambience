#pragma once

#include <tos/gui/element.hpp>
#include <tos/math/saturated.hpp>

namespace tos::gui::decorators {
template<class Base>
struct margin {
    Base base;
    int left, right, top, bottom;

    view_limits limits(const draw_context& ctx) const {
        view_limits base_size = base.limits(ctx);
        return view_limits{{math::add_saturating(base_size.min.width(), left + right),
                            math::add_saturating(base_size.min.height(), top + bottom)},
                           {math::add_saturating(base_size.max.width(), left + right),
                            math::add_saturating(base_size.max.height(), top + bottom)}};
    }

    void draw(const draw_context& ctx) {
        tos::gfx2::size base_size = ctx.bounds.dims();
        auto actual_size = tos::gfx2::size{base_size.width() - left - right,
                                           base_size.height() - top - bottom};

        auto new_ctx = ctx;
        new_ctx.bounds.corner().x() += left;
        new_ctx.bounds.corner().y() += top;
        new_ctx.bounds.dims() = actual_size;

        base.draw(new_ctx);
    }
};

template<class Base>
margin(Base&& base, int left, int right, int top, int bottom) -> margin<Base>;

template<class Base>
struct bordered {
    Base base;

    view_limits limits(const draw_context& ctx) {
        return base.limits(ctx);
    }

    void draw(const draw_context& ctx) {
        base.draw(ctx);

        tos::services::style s(ctx.theme->fg_color);
        ctx.painter->set_style(s);
        ctx.painter->draw_rect(ctx.bounds, 0, false);
    }
};

template<class Base>
bordered(Base&& b) -> bordered<Base>;

template<class Base>
struct round_bordered {
    int radius;
    Base base;

    view_limits limits(const draw_context& ctx) const {
        return base.limits(ctx);
    }

    void draw(const draw_context& ctx) {
        base.draw(ctx);

        tos::services::style s(tos::gfx2::binary_color{false});
        ctx.painter->set_style(s);
        ctx.painter->draw_rect(ctx.bounds, radius, false);
    }
};

template<class Base>
round_bordered(int radius, Base&& b) -> round_bordered<Base>;

template<class Base>
struct fixed_size {
    fixed_size(Base&& b, const tos::gfx2::size& sz)
        : m_base{std::move(b)}
        , m_lims{sz, sz} {
    }

    view_limits limits(const draw_context& ctx) const {
        return m_lims;
    }

    void draw(const draw_context& ctx) {
        auto new_ctx = ctx;
        new_ctx.bounds.dims() = m_lims.min;
        m_base.draw(new_ctx);
    }

private:
    Base m_base;
    view_limits m_lims;
};

template<class Base>
fixed_size(Base&& b, const tos::gfx2::size&) -> fixed_size<Base>;

template<class Base>
struct horizontal_center_align {
    Base base;

    view_limits limits(const draw_context& ctx) {
        auto rect = compute_rect(ctx);
        return {rect.dims(), rect.dims()};
    }

    void draw(const draw_context& ctx) {
        auto new_ctx = ctx;
        new_ctx.bounds = compute_rect(ctx);
        base.draw(new_ctx);
    }

private:
    gfx2::rectangle compute_rect(const draw_context& ctx) {
        const view_limits& base_limits = base.limits(ctx);

        auto available_width = ctx.bounds.dims().width();

        auto elem_width = base_limits.min.width();

        if (available_width > elem_width) {
            elem_width = std::min(available_width, base_limits.max.width());
        }

        auto x_diff = (ctx.bounds.dims().width() - elem_width) / 2;

        return gfx2::rectangle{
            {ctx.bounds.corner().x() + x_diff, ctx.bounds.corner().y()},
            {elem_width, std::min(ctx.bounds.dims().height(), base_limits.max.height())}};
    }
};
template<class Base>
horizontal_center_align(Base&& b) -> horizontal_center_align<Base>;

template<class Base>
struct vertical_center_align {
    Base base;

    view_limits limits(const draw_context& ctx) {
        auto rect = compute_rect(ctx);
        return {rect.dims(), rect.dims()};
    }

    void draw(const draw_context& ctx) {
        auto new_ctx = ctx;
        new_ctx.bounds = compute_rect(ctx);
        base.draw(new_ctx);
    }

private:
    gfx2::rectangle compute_rect(const draw_context& ctx) {
        const view_limits& base_limits = base.limits(ctx);

        auto available_height = ctx.bounds.dims().height();

        auto elem_height = base_limits.min.height();

        if (available_height > elem_height) {
            elem_height = std::min(available_height, base_limits.max.height());
        }

        auto y_diff = (ctx.bounds.dims().height() - elem_height) / 2;

        LOG(ctx.bounds.dims().width(), base_limits.min.width(), base_limits.max.width());
        return gfx2::rectangle{
            {ctx.bounds.corner().x(), ctx.bounds.corner().y() + y_diff},
            {ctx.bounds.dims().width(), elem_height}};
    }
};

template<class Base>
vertical_center_align(Base&& b) -> vertical_center_align<Base>;

template<class BaseT>
auto align_center_middle(BaseT&& b) {
    return vertical_center_align{horizontal_center_align{std::forward<BaseT>(b)}};
}
} // namespace tos::gui::decorators