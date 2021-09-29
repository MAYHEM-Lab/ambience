//
// Created by fatih on 11/7/19.
//

#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <tos/function_ref.hpp>
#include <tos/span.hpp>

namespace tos::shell {
class dynamic_command_storage {
public:
    void add_command(std::string name,
                     tos::function_ref<void(tos::span<std::string_view>)> fn) {
        m_storage.emplace(std::move(name), fn);
    }

    auto get_command(const std::string_view& name)
        -> std::optional<tos::function_ref<void(tos::span<std::string_view>)>> {
        auto it = m_storage.find(name);
        if (it == m_storage.end()) {
            return std::nullopt;
        }
        return it->second;
    }

private:
    std::map<std::string,
             tos::function_ref<void(tos::span<std::string_view>)>,
             std::less<>>
        m_storage;
};
} // namespace tos::shell