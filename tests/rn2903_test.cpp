//
// Created by fatih on 12/11/18.
//

#include <tos/span.hpp>
#include "catch.hpp"
#include <string>
#include <chrono>
#include <cstring>
#include <drivers/common/rn2903/sys.hpp>

using namespace tos;

struct uart_driver
{
    int write(tos::span<const char> buf)
    {
        auto str = std::string(buf.begin(), buf.end());
        m_buf += str;

        auto index = m_buf.find("\r\n");
        if (index == std::string::npos)
        {
            return buf.size();
        }

        REQUIRE(m_buf.substr(0, index + 2) == "sys get nvm 299\r\n");
        return buf.size();
    }

    template <class AlarmT>
    span<char> read(span<char> data, AlarmT& alarm, std::chrono::milliseconds timeout)
    {
        REQUIRE(data.size() >= 20);
        auto r = std::strncpy(data.begin(), "invalid_param\r\n", data.size());
        return data.slice(0, std::distance(data.begin(), r));
    }

private:
    std::string m_buf;
};

struct alarm_mock {};

TEST_CASE("rn2903 nvm get", "[rn2903,get]")
{
    uart_driver dr;
    alarm_mock al;

    auto res = rn2903::nvm_get({299}, dr, al);
    REQUIRE(!res);
}