#pragma once

#include <csetjmp>
#include <fast_float/fast_float.h>
#include <lidlrt/concepts.hpp>
#include <lidlrt/string.hpp>

namespace tos::ae {
namespace detail {
template<class T>
using ReturnT = std::conditional_t<lidl::Value<T>, T, T&>;

template<class T>
requires(std::is_floating_point_v<std::remove_cvref_t<T>> ||
         std::is_integral_v<std::remove_cvref_t<T>>)
    std::remove_cvref_t<T> try_translate(const jsmn::parser& parser,
                                               lidl::message_builder& mb,
                                               jmp_buf err_handler) {
    double res;
    const auto text = parser.text(parser.front());
    auto parse_res = fast_float::from_chars(text.data(), text.data() + text.size(), res);
    if (parse_res.ec != std::errc{}) {
        longjmp(err_handler, 3);
    }
    return static_cast<std::remove_cvref_t<T>>(res);
}

template<class T>
requires(std::is_same_v<std::string_view, std::remove_cvref_t<T>>)
    std::remove_cvref_t<T> try_translate(const jsmn::parser& parser,
                                               lidl::message_builder& mb,
                                               jmp_buf err_handler) {
    return parser.text(parser.front());
}

template<class T>
requires(std::is_same_v<lidl::string, std::remove_cvref_t<T>>)
    lidl::string& try_translate(const jsmn::parser& parser,
                                      lidl::message_builder& mb,
                                      jmp_buf err_handler) {
    auto text = parser.text(parser.front());
    return lidl::create_string(mb, text);
}


template<lidl::Struct ParamStructT>
ReturnT<ParamStructT> try_translate(const jsmn::object_parser& parser,
                                           lidl::message_builder& mb,
                                           jmp_buf err_handler);

template<lidl::Union CallUnionT>
ReturnT<CallUnionT> try_translate(const jsmn::object_parser& parser,
                                        lidl::message_builder& mb,
                                        jmp_buf err_handler);

template<class T>
decltype(auto) try_translate_param(const jsmn::object_parser& parser,
                                   std::string_view name,
                                   lidl::message_builder& mb,
                                   jmp_buf err_handler) {
    auto val_opt = parser.value(name);
    if (!val_opt) {
        longjmp(err_handler, 4);
    }
    return try_translate<T>(*val_opt, mb, err_handler);
}

template<lidl::Struct ParamStructT>
ReturnT<ParamStructT> try_translate(const jsmn::object_parser& parser,
                                           lidl::message_builder& mb,
                                           jmp_buf err_handler) {
    using traits_type = ::lidl::struct_traits<ParamStructT>;

    auto fn = [&]<class... Ts>(const Ts&... mems)->decltype(auto) {
        if constexpr (lidl::Value<ParamStructT>) {
            return ParamStructT(try_translate_param<typename lidl::procedure_traits<
                                    decltype(mems.function)>::return_type>(
                parser, mems.name, mb, err_handler)...);
        } else {
            return lidl::create<ParamStructT>(
                mb,
                try_translate_param<typename lidl::procedure_traits<
                    decltype(mems.function)>::return_type>(
                    parser, mems.name, mb, err_handler)...);
        }
    };

    return std::apply(fn, traits_type::members);
}

template<lidl::Union CallUnionT, lidl::Struct ParamsStructT>
ReturnT<CallUnionT> wrap_one(const jsmn::object_parser& parser,
                             lidl::message_builder& mb,
                             jmp_buf err_handler) {
    if constexpr (lidl::Value<CallUnionT>) {
        return CallUnionT(try_translate<ParamsStructT>(parser, mb, err_handler));
    } else {
        return lidl::create<CallUnionT>(
            mb, try_translate<ParamsStructT>(parser, mb, err_handler));
    }
};

template<lidl::Union CallUnionT>
ReturnT<CallUnionT> try_translate(const jsmn::object_parser& parser,
                                        lidl::message_builder& mb,
                                        jmp_buf err_handler) {
    using traits_type = ::lidl::union_traits<CallUnionT>;
    using fn_ptr_t = ReturnT<CallUnionT> (*)(
        const jsmn::object_parser&, lidl::message_builder& mb, jmp_buf err_handler);

    constexpr auto make_table = []<class... Ts>(lidl::meta::list<Ts...>) {
        return std::array<fn_ptr_t, sizeof...(Ts)>{&wrap_one<CallUnionT, Ts>...};
    };

    constexpr auto jump_table = make_table(typename traits_type::types{});
    auto index_opt = parser.value("index");
    if (!index_opt) {
        longjmp(err_handler, 1);
    }

    auto index = try_translate<int>(*index_opt, mb, err_handler);
    if (index > static_cast<int>(jump_table.size()) || index < 0) {
        longjmp(err_handler, 2);
    }

    return jump_table[index](parser, mb, err_handler);
}
} // namespace detail
} // namespace tos::ae