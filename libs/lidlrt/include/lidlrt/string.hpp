#pragma once

#include "ptr.hpp"

#include <lidlrt/buffer.hpp>
#include <lidlrt/builder.hpp>
#include <lidlrt/traits.hpp>
#include <string_view>

namespace lidl {
class string {
public:
    explicit string(int16_t len)
        : m_len(len) {
    }

    string(const string&) = delete;
    string(string&&)      = delete;

//    [[nodiscard]] std::string_view string_view() const {
//        auto& raw = m_cur_pos.unsafe().get();
//        return raw.string_view();
//    }

    operator std::string_view() const {
        return string_view();
    }

    [[nodiscard]] std::string_view string_view() const {
        auto begin = data();
        return {begin, size_t(m_len)};
    }
private:
    const char* data() const {
        auto potential_begin = (&m_len) + 1; // the string begins after the length.
        return reinterpret_cast<const char*>(potential_begin);
    }

    int16_t m_len;
    // char data[];
};

inline bool operator==(const string& left, const string& right) {
    return left.string_view() == right.string_view();
}

static_assert(sizeof(string) == 2);
static_assert(alignof(string) == 2);

template<>
struct is_reference_type<string> : std::true_type {};

inline string& create_string(message_builder& builder, int len) {
    auto& inserted_len = emplace_raw<string>(builder, int16_t(len));
    builder.allocate(len, 1); // stores the string body
    return inserted_len;
}

inline string& create_string(message_builder& builder, std::string_view sv) {
    auto& str = create_string(builder, static_cast<int>(sv.size()));
    std::copy(sv.begin(), sv.end(), const_cast<char*>(str.string_view().data()));
    return str;
}
} // namespace lidl