#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#define JSMN_HEADER
#include "jsmn.h"

#if defined(__cplusplus)
}
#endif

#include <optional>
#include <string_view>
#include <tos/span.hpp>
#include <vector>

namespace jsmn {
inline
std::optional<std::vector<jsmntok_t>> parse(std::string_view json) {
    jsmn_parser p;
    jsmn_init(&p);

    auto r = jsmn_parse(&p, json.data(), json.size(), nullptr, 0);

    if (r < 0) {
        return std::nullopt;
    }

    jsmn_init(&p);

    std::vector<jsmntok_t> toks(r);
    r = jsmn_parse(&p, json.data(), json.size(), toks.data(), toks.size());
    return toks;
}

struct parser {
    tos::span<const jsmntok_t> tokens;
    std::string_view body;

    const jsmntok_t& front() const {
        return tokens.front();
    }

    const jsmntok_t& pop_front() {
        auto& res = tokens.front();
        tokens = tokens.pop_front();
        return res;
    }

    void consume_whole() {
        int to_consume = 1;

        while (to_consume-- > 0) {
            auto& front = pop_front();
            switch (front.type) {
            case JSMN_OBJECT:
                to_consume += front.size * 2;
                break;
            case JSMN_ARRAY:
                to_consume += front.size;
                break;
            case JSMN_UNDEFINED:
            case JSMN_STRING:
            case JSMN_PRIMITIVE:
                break;
            }
        }
    }

    std::string_view text(const jsmntok_t& tok) const {
        return body.substr(tok.start, tok.end - tok.start);
    }
};

struct object_parser {
    parser m_parser;

    template<class FnT>
    void for_each(FnT&& fn) const {
        auto sz = m_parser.front().size;
        parser p = m_parser;
        p.pop_front();
        for (int i = 0; i < sz; ++i) {
            auto key = p.text(p.pop_front());
            if (fn(key, static_cast<const parser&>(p))) {
                break;
            }
            p.consume_whole();
        }
    }

    bool has_key(std::string_view key) const {
        bool res = false;
        for_each([&](std::string_view el_key, const auto& val_parser) {
            if (key == el_key) {
                res = true;
                return true;
            }
            return false;
        });
        return res;
    }

    size_t size() const {
        return m_parser.front().size;
    }

    std::optional<parser> value(std::string_view key) const {
        std::optional<parser> res;
        for_each([&](std::string_view el_key, const auto& val_parser) {
            if (key == el_key) {
                res = val_parser;
                return true;
            }
            return false;
        });
        return res;
    }
};
} // namespace jsmn
