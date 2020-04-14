//
// Created by fatih on 12/11/18.
//

#include "doctest.h"

#include <chrono>
#include <common/driver_base.hpp>
#include <common/rn2903/sys.hpp>
#include <cstring>
#include <string>
#include <tos/span.hpp>

using namespace tos;

struct uart_driver : self_pointing<uart_driver> {
    int write(tos::span<const uint8_t> buf) {
        auto str = std::string(buf.begin(), buf.end());
        m_buf += str;

        auto index = m_buf.find("\r\n");
        if (index == std::string::npos) {
            return buf.size();
        }

        REQUIRE_EQ("sys get nvm 299\r\n", m_buf.substr(0, index + 2));
        return buf.size();
    }

    template<class AlarmT>
    span<uint8_t>
    read(span<uint8_t> data, AlarmT& alarm, std::chrono::milliseconds timeout) {
        REQUIRE_LE(20, data.size());
        auto r =
            std::strncpy(reinterpret_cast<char*>(data.begin()), "FF\r\n", data.size());
        return data.slice(0, std::distance(reinterpret_cast<char*>(data.begin()), r));
    }

private:
    std::string m_buf;
};

struct alarm_mock {};

TEST_CASE("rn2903 nvm get") {
    uart_driver dr;
    alarm_mock al;

    auto res = rn2903::nvm_get({299}, dr, al);
    REQUIRE(!res);
}