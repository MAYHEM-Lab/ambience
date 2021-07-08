#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/write.hpp>
#include <common/usart.hpp>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos::hosted {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

struct usart : self_pointing<usart> {
public:
    usart(boost::asio::io_service& io,
          std::string_view serial_name,
          usart_constraint&& config)
        : m_port(io, std::string(serial_name)) {
        namespace asio = boost::asio;
        m_port.set_option(
            asio::serial_port::baud_rate(tos::get<usart_baud_rate>(config).rate));
        m_port.set_option(asio::serial_port::parity(asio::serial_port::parity::none));
        m_port.set_option(
            asio::serial_port::stop_bits(asio::serial_port::stop_bits::one));
        m_port.set_option(
            asio::serial_port::flow_control(asio::serial_port::flow_control::none));
    }

    int write(span<const uint8_t> buf) {
        int size = 0;
        boost::system::error_code ec;
        boost::asio::async_write(m_port,
                                 boost::asio::buffer(buf.data(), buf.size()),
                                 [&](boost::system::error_code e, std::size_t len) {
                                     size = len;
                                     ec = e;
                                     m_write.up();
                                 });
        m_write.down();
        return buf.size();
    }

    span<uint8_t> read(span<uint8_t> buf) {
        int size = 0;
        boost::system::error_code ec;
        boost::asio::async_read(m_port,
                                boost::asio::buffer(buf.data(), buf.size()),
                                [&](boost::system::error_code e, std::size_t len) {
                                    size = len;
                                    ec = e;
                                    m_read.up();
                                });
        m_read.down();
        if (ec) {
            return buf.slice(0, size);
        }
        return buf.slice(0, size);
    }

    template<class AlarmT>
    span<uint8_t> read(span<uint8_t> buf, AlarmT& alarm, std::chrono::milliseconds to) {
        int size = 0;
        boost::system::error_code ec;
        boost::asio::async_read(m_port,
                                boost::asio::buffer(buf.data(), buf.size()),
                                [&](boost::system::error_code e, std::size_t len) {
                                    size = len;
                                    ec = e;
                                    m_read.up();
                                });
        auto res = m_read.down(alarm, to);
        if (res != sem_ret::normal) {
            m_port.cancel();
            m_read.down();
        }
        if (ec) {
            return buf.slice(0, size);
        }
        return buf.slice(0, size);
    }

    void close() {
        m_port.close();
    }

    ~usart() {
        close();
    }

private:
    semaphore m_read{0};
    semaphore m_write{0};
    boost::asio::serial_port m_port;
};
} // namespace tos::hosted
