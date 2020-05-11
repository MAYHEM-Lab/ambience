//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <boost/asio/posix/stream_descriptor.hpp>
#include <common/driver_base.hpp>
#include <common/tty.hpp>
#include <common/usart.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

#undef putc

namespace tos {
namespace hosted {
class stderr_adapter : public self_pointing<stderr_adapter> {
public:
    int write(span<const uint8_t> buf);
};

class stdio : public self_pointing<stdio> {
public:
    stdio();

    int write(span<const uint8_t> buf);
    span<uint8_t> read(span<uint8_t> buf);

private:
    boost::asio::posix::stream_descriptor m_input;
};
} // namespace hosted

template<class T>
inline hosted::stdio open_impl(devs::usart_t<0>, T) {
    return {};
}

inline hosted::stdio open_impl(devs::tty_t<0>) {
    return {};
}
} // namespace tos
