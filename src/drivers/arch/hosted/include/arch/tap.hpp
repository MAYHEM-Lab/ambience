#pragma once

#include <boost/asio/posix/stream_descriptor.hpp>
#include <memory>
#include <tos/expected.hpp>
#include <tos/span.hpp>
#include <tos/platform.hpp>

namespace tos::hosted {
struct error_code {
    int m_code;
};

/**
 * This driver implements a virtual network device on supported systems using
 * tap drivers.
 */
struct tap_device {
    explicit tap_device(int fd)
        : m_input{std::make_unique<boost::asio::posix::stream_descriptor>(get_io(), fd)} {
    }

    tap_device(tap_device&&) = default;

    /**
     * Writes a raw link layer frame to the network.
     */
    int write(span<const uint8_t> data);

    /**
     * Attempts to read a link layer frame from the network.
     * If the buffer is large enough, the resulting span will have the entire frame.
     * If the buffer is not large enough, it will perform a short read, and the rest of the 
     * frame must be read in the future.
     */
    span<uint8_t> read(span<uint8_t> data);

    ~tap_device();

private:
    std::unique_ptr<boost::asio::posix::stream_descriptor> m_input;
};

expected<tap_device, error_code> make_tap_device();
} // namespace tos::hosted