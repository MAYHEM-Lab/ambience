//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#include <arch/stdio.hpp>
#include <boost/asio/read.hpp>
#include <iostream>

namespace tos::x86 {
stdio::stdio()
    : m_input{get_io(), ::dup(STDIN_FILENO)} {
}

int stdio::write(span<const uint8_t> buf) {
    ::std::cout.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    ::std::cout.flush();
    return buf.size();
}

span<uint8_t> stdio::read(span<uint8_t> buf) {
    int size = 0;
    tos::semaphore wait_sem{0};
    boost::system::error_code ec;
    boost::asio::async_read(m_input,
                            boost::asio::buffer(buf.data(), buf.size()),
                            [&](boost::system::error_code e, std::size_t len) {
                                size = len;
                                ec = e;
                                wait_sem.up();
                            });
    wait_sem.down();
    if (ec) {
        LOG_WARN("Error", ec.message());
    }
    return buf.slice(0, size);
}

int stderr_adapter::write(span<const uint8_t> buf) {
    ::std::cerr.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    ::std::cerr.flush();
    return buf.size();
}
} // namespace tos