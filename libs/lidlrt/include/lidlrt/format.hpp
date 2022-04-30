#include "lidlrt/structure.hpp"
#include <fmt/compile.h>
#include <fmt/format.h>
#include <lidlrt/concepts.hpp>
#include <lidlrt/lib.hpp>
#include <tos/meta/algorithm.hpp>

namespace fmt {
template<lidl::Struct T>
struct formatter<T> {
    using traits = lidl::struct_traits<T>;

    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    struct compile_string_t : fmt::detail::compiled_string {
        static constexpr auto make_fmt_str() {
            constexpr auto names = tos::meta::transform(traits::members, [](auto& mem) {
                return tos::fixed_string("\"") + mem.name + tos::fixed_string("\"");
            });
            constexpr auto parts = tos::meta::transform(names, [](auto& name) {
                return name + tos::fixed_string(": {}, ");
            });
            constexpr auto total = tos::meta::accumulate(parts, tos::fixed_string("{{")) + tos::fixed_string("}}");
            
            std::array<char, total.size()> ret{};
            std::copy(std::begin(total.val), std::end(total.val), ret.begin());
            return ret;
        }

        static constexpr auto fmt_str = make_fmt_str();

        using char_type = char;

        [[maybe_unused]] constexpr explicit
        operator fmt::basic_string_view<char>() const {
            return fmt::basic_string_view<char>(fmt_str.begin(), fmt_str.size());
        }
    };

    static constexpr auto format_string() {
        return compile_string_t{};
    }

    template<typename FormatContext>
    constexpr auto format(T const& str, FormatContext& ctx) {
        using lidl::apply;
        return apply([&](const auto&... mems) {
            return fmt::format_to(ctx.out(), format_string(), mems...);
        }, str);
    }
};
} // namespace fmt