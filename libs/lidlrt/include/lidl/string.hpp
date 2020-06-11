#pragma once

#include "ptr.hpp"

#include <lidl/buffer.hpp>
#include <lidl/builder.hpp>
#include <string_view>
#include <lidl/traits.hpp>

namespace lidl {
class string {
public:
    explicit string(uint8_t* data)
        : m_ptr(data) {
    }

    string(const string&) = delete;
    string(string&&) = delete;

    [[nodiscard]]
    std::string_view string_view() const {
        return {&m_ptr.unsafe().get(), static_cast<size_t>(m_ptr.get_offset())};
    }

    operator std::string_view() const {
        return string_view();
    }

private:
    ptr<char> m_ptr{nullptr};
};

static_assert(sizeof(string) == 2);
static_assert(alignof(string) == 2);

template <> struct is_reference_type<string> : std::true_type {};

inline string& create_string(message_builder& builder, int len) {
    auto padding = (builder.size() + len) % alignof(string);
    builder.allocate(padding, 1);
    auto alloc = builder.allocate(len, 1);
    return emplace_raw<string>(builder, alloc);
}

inline string& create_string(message_builder& builder, std::string_view sv) {
    auto& str = create_string(builder, sv.size());
    std::copy(sv.begin(), sv.end(), const_cast<char*>(str.string_view().data()));
    return str;
}
} // namespace lidl