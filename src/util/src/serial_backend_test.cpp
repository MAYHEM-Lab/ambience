#include <tos/io/serial_backend.hpp>
#include <doctest.h>
#include <deque>

namespace tos::io {
namespace {
struct mock_fifo {
    span<uint8_t> read(span<uint8_t> s) {
        auto read_len = std::min(s.size(), buf.size());
        s = s.slice(0, read_len);
        std::copy(buf.begin(), buf.begin() + s.size(), s.begin());
        buf.erase(buf.begin(), buf.begin() + s.size());
        return s;
    }

    int write(span<const uint8_t> s) {
        buf.insert(buf.end(), s.begin(), s.end());
        return s.size();
    }

public:
    std::deque<uint8_t> buf;
};

TEST_CASE("Serial backend try_one works provided send works") {
    mock_fifo fifo;
    serial_backend backend(&fifo);
    auto msg = raw_cast(span("hello"));

    backend.send(0, msg);

    auto res = backend.receive_one();
    REQUIRE(res.has_value());

    auto& [streamid, packet] = force_get(res);

    REQUIRE_EQ(0, streamid);
    REQUIRE_EQ(6, packet->size());
    REQUIRE_EQ(msg, as_span(*packet));
}
}
}