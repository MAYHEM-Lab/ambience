//
// Created by fatih on 12/11/18.
//

#include <tos/span.hpp>
#include "doctest.h"
#include <string>
#include <chrono>
#include <cstring>
#include <common/rn2903/sys.hpp>
#include <common/driver_base.hpp>

using namespace tos;

struct uart_driver : self_pointing<uart_driver>
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

        REQUIRE(m_buf.substr(0, index + 2) == "sys get nvm 300\r\n");
        return buf.size();
    }

    template <class AlarmT>
    span<char> read(span<char> data, AlarmT& alarm, std::chrono::milliseconds timeout)
    {
        REQUIRE(data.size() >= 20);
        auto r = std::strncpy(data.begin(), "FF\r\n", data.size());
        return data.slice(0, std::distance(data.begin(), r));
    }
private:
    std::string m_buf;
};

struct alarm_mock {};

TEST_CASE("rn2903 nvm get")
{
    uart_driver dr;
    alarm_mock al;

    auto res = rn2903::nvm_get({299}, dr, al);
    REQUIRE(!res);
}