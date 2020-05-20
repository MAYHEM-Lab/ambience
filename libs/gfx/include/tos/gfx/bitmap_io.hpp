#pragma once

#include <tos/gfx/bitmap.hpp>
#include <tos/gfx/color.hpp>
#include <tos/print.hpp>

namespace tos::gfx {
template<class OutT>
void serialize_pgm(basic_bitmap_view<mono8> bitmap, OutT& out) {
    tos::println(out, "P2");
    tos::println(out, bitmap.dims().width, bitmap.dims().height);
    for (int row = 0; row < bitmap.dims().height; ++row) {
        for (auto& col : bitmap.row(row)) {
            tos::print(out, int(col.bw), "");
        }
        tos::println(out);
    }
}
}