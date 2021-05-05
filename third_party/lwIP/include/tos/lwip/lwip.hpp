//
// Created by fatih on 8/4/18.
//

#pragma once

#include <algorithm>
#include <cstddef>
#include <lwip/pbuf.h>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

namespace tos {
namespace lwip {
/**
 * Lwip events that must be handled by the users of `tcp_endpoint` object.
 */
namespace events {
static constexpr struct recvfrom_t {
} recvfrom{};
static constexpr struct recv_t {
} recv{};
static constexpr struct sent_t {
} sent{};
static constexpr struct discon_t { } discon{}; } // namespace events

enum class connect_error
{
    not_connected,
    cert_invalid,
    ssl_error,
    no_network,
    connection_reset,
    unknown
};

/**
 * This enumeration contains the reasons for a TCP connection to
 * disconnect.
 */
enum class discon_reason
{
    /**
     * The connection closed gracefully, either by us or the other endpoint.
     */
    closed,
    /**
     * There was an error in the SSL decryption of the received packet.
     */
    ssl_error,
    /**
     * Lwip receive callback returned a non-ok status.
     */
    recv_error,
    /**
     * A send operation failed. Not used right now.
     */
    send_error,
    /**
     * Connection was aborted
     */
    aborted,
    /**
     * Connection was reset
     */
    reset,
    /**
     * Unknown cause
     */
    other_error
};

/**
 * This class represents an lwip pbuf chain. It manages partial reads and
 * chains internally, and exposes a linear read interface.
 */
struct buffer {
    /**
     * Creates an empty buffer object.
     */
    buffer()
        : m_root{nullptr}
        , m_read_off{0} {
    }

    /**
     * Creates a buffer object from the given pbuf.
     * The new buffer takes ownership of the given pbuf, thus `pbuf_free` shouldn't
     * be called on the passed pbuf.
     *
     * @param buf buffer to take ownership of.
     */
    explicit buffer(pbuf* buf)
        : m_root{buf}
        , m_read_off{0}
        , m_len(buf->tot_len) {
    }

    buffer(const buffer&) = delete;

    buffer(buffer&& rhs) noexcept
        : m_root{rhs.m_root}
        , m_read_off{rhs.m_read_off}
        , m_len(get_count(rhs.m_len)) {
        rhs.m_root = nullptr;
        reset(rhs.m_len, 0);
    }

    buffer& operator=(buffer&& rhs) noexcept {
        if (m_root) {
            pbuf_free(m_root);
        }

        auto len = get_count(rhs.m_len);
        reset(rhs.m_len, 0);
        m_root = std::exchange(rhs.m_root, nullptr);
        m_read_off = rhs.m_read_off;
        up_many(m_len, len);
        return *this;
    }

    ~buffer() {
        if (m_root) {
            pbuf_free(m_root);
        }
    }

    void eof() {
        m_len.up();
    }

    /**
     * Reads up to buf.size() bytes from the current pbuf. If the current
     * pbuf has less than buf.size() bytes, a short read will occur.
     *
     * @param buf buffer to read to
     * @return the bytes that are read
     */
    span<uint8_t> read(span<uint8_t> buf) {
        m_len.down();
        m_len.up();

        auto remaining = std::min(buf.size(), size());

        auto total = 0;
        for (auto out_it = buf.begin(); remaining > 0;) {
            auto bucket = cur_bucket();
            auto bucket_sz = std::min(bucket.size(), remaining);
            std::copy(bucket.begin(), bucket.begin() + bucket_sz, out_it);
            out_it += bucket_sz;
            total += bucket_sz;
            remaining -= bucket_sz;
            consume(bucket_sz);
        }

        return buf.slice(0, total);
    }

    /**
     * Returns the total number of bytes in the whole chain
     * @return number of bytes
     */
    size_t size() const {
        if (!m_root)
            return 0;
        return m_root->tot_len - m_read_off;
    }

    /**
     * Returns whether there exists at least one pbuf object
     * in the chain.
     * @return whether a read can be issued right now
     */
    bool has_more() const {
        return m_root;
    }

    /**
     * Appends the given buffer to this buffer, chaining the
     * internal pbuf objects and releasing the pbuf of the given
     * buffer.
     *
     * @param b buffer to append to this one
     */
    void append(buffer&& b) {
        auto buf = b.m_root;
        pbuf_cat(m_root, buf);
        up_many(m_len, b.size());
        b.m_root = nullptr;
        reset(b.m_len, 0);
    }

private:
    /**
     * Consumes the given number of bytes from the pbuf chain
     *
     * Number of bytes must be less than or equal to the cur_bucket().size()
     *
     * @param sz number of bytes
     */
    void consume(size_t sz) {
        for (size_t i = 0; i < sz; ++i) {
            m_len.down();
        }

        if (sz < cur_bucket().size()) {
            m_read_off += sz;
            return;
        }

        if (!m_root->next) {
            // done
            pbuf_free(m_root);
            m_root = nullptr;
            return;
        }

        auto next = m_root->next;
        pbuf_ref(next);
        pbuf_free(m_root);
        m_root = next;
        m_read_off = 0;
    }

    span<uint8_t> cur_bucket() {
        return {(uint8_t*)m_root->payload + m_read_off, m_root->len - m_read_off};
    }

    span<const uint8_t> cur_bucket() const {
        return {(const uint8_t*)m_root->payload + m_read_off, m_root->len - m_read_off};
    }

    pbuf* m_root;
    size_t m_read_off;
    tos::semaphore m_len{0};
};

inline tos::recursive_mutex lwip_lock;
} // namespace lwip
} // namespace tos