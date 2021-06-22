#include <arch/tap.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <tos/debug/assert.hpp>
#include <tos/debug/panic.hpp>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>
#include <unistd.h>

namespace tos::hosted {
expected<tap_device, error_code> make_tap_device() {
    int tapfd = ::open("/dev/net/tun", O_RDWR);
    if (tapfd < 0) {
        LOG_ERROR(strerror(errno));
        return unexpected(error_code{errno});
    }

    ifreq ifr{};
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strcpy(ifr.ifr_name, "tap0");

    int err = ioctl(tapfd, TUNSETIFF, &ifr);
    if (err < 0) {
        LOG_ERROR(strerror(errno));
        return unexpected(error_code{errno});
    }

    LOG_TRACE("Got the tap device");

    mac_addr_t addr;
    ifreq s;
    if (ioctl(tapfd, SIOCGIFHWADDR, &s) == 0) {
        std::copy(s.ifr_ifru.ifru_addr.sa_data,
                  s.ifr_ifru.ifru_addr.sa_data + 6,
                  addr.addr.data());
    }

    return tap_device{tapfd, addr};
}

int tap_device::write(span<const uint8_t> buf) {
    tos::semaphore wait_write{0};
    boost::asio::async_write(
        *m_input,
        boost::asio::buffer(buf.data(), buf.size()),
        [&](boost::system::error_code, std::size_t) { wait_write.up(); });
    wait_write.down();

    return buf.size();
}

span<uint8_t> tap_device::read(span<uint8_t> buf) {
    tos::semaphore wait_read{0};
    int size = 0;
    boost::system::error_code ec;
    m_input->async_read_some(boost::asio::buffer(buf.data(), buf.size()),
                             [&](boost::system::error_code e, std::size_t len) {
                                 size = len;
                                 ec = e;
                                 wait_read.up();
                             });
    wait_read.down();
    if (ec) {
        tos::debug::panic(ec.message().c_str());
    }
    return buf.slice(0, size);
}

tap_device::~tap_device() = default;
} // namespace tos::hosted