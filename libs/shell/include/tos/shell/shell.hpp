//
// Created by fatih on 11/7/19.
//

#pragma once

#include "detail/echoing_uart.hpp"

#include "dynamic_command_storage.hpp"
#include <algorithm>
#include <tos/print.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>
#include <tos/streams.hpp>
#include <utility>

namespace tos::shell {
template<size_t Size>
struct static_line_buffer_storage {
    std::array<uint8_t, Size> m_storage;
    auto& get_line_storage() {
        return m_storage;
    }
};

template<class StreamT, class LineBufferStorage = static_line_buffer_storage<64>>
class shell : private LineBufferStorage {
public:
    template<class... StorageArgs>
    explicit shell(StreamT str, StorageArgs&&... args)
        : m_stream{std::move(str)}
        , LineBufferStorage{std::forward<StorageArgs>(args)...} {
    }

    void run() {
        while (!m_exit) {
            handle_one();
        }
    }

    auto& get_command_store() {
        return m_storage;
    }

private:
    void handle_one() {
        tos::print(m_stream, "$ ");

        detail::echoing_uart echo{m_stream};

        auto& line_buffer = LineBufferStorage::get_line_storage();
        char sep[] = {'\r', '\n'};
        auto cmd = tos::read_until<char>(
            echo, tos::span(sep), tos::raw_cast<char>(tos::span(line_buffer)));
        if (cmd.size() == line_buffer.size() &&
            !(cmd.slice(cmd.size() - 2, 2) == tos::span(sep))) {
            // command didn't fit!
            tos::println(m_stream, "Command line too big!");
            return;
        }

        cmd = cmd.slice(0, cmd.size() - std::size(sep));
        if (cmd.empty()) {
            return;
        }
        std::vector<std::string_view> args;
        for (auto beg = cmd.begin(); beg != cmd.end();) {
            auto first_space = std::find(beg, cmd.end(), ' ');
            auto elem = tos::span<const char>(beg, first_space);
            if (!elem.empty() && !(elem == tos::span(sep))) {
                args.emplace_back(elem.begin(), elem.size());
            }
            if (first_space == cmd.end()) {
                break;
            }
            beg = first_space + 1;
        }

        auto handler = m_storage.get_command(args[0]);
        if (!handler) {
            auto span = tos::span<const char>(args[0].begin(), args[0].end());
            tos::println(m_stream, "Unknown command", span);
            return;
        }

        (*handler)(args);
    }

    bool m_exit = false;
    StreamT m_stream;
    dynamic_command_storage m_storage;
};
} // namespace tos::shell