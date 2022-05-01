#include "lidlrt/structure.hpp"
#include <fmt/compile.h>
#include <fmt/format.h>
#include <lidlrt/array.hpp>
#include <lidlrt/concepts.hpp>
#include <lidlrt/lib.hpp>
#include <lidlrt/ptr.hpp>
#include <lidlrt/vector.hpp>
#include <tos/meta/algorithm.hpp>

namespace fmt {
template<class T>
struct formatter<lidl::vector<T>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(lidl::vector<T> const& vec, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), FMT_COMPILE("[{}]"), fmt::join(vec, ", "));
    }
};

template<class T, size_t N>
struct formatter<lidl::array<T, N>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(lidl::array<T, N> const& arr, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), FMT_COMPILE("[{}]"), fmt::join(arr, ", "));
    }
};

template<class T>
struct formatter<lidl::ptr<T>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(lidl::ptr<T> const& ptr, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), FMT_COMPILE("{}"), ptr.unsafe().get());
    }
};

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
                return tos::fixed_string("\"") + mem.name + tos::fixed_string("\": {}");
            });
            constexpr auto total =
                tos::meta::join(names, tos::fixed_string("{{"), tos::fixed_string(", ")) +
                tos::fixed_string("}}");

            std::array<char, total.size() - 1> ret{};
            std::copy(std::begin(total.val), std::end(total.val) - 1, ret.begin());
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
        return apply(
            [&](const auto&... mems) {
                return fmt::format_to(ctx.out(), format_string(), mems...);
            },
            str);
    }
};
} // namespace fmt