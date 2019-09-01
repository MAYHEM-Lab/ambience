//
// Created by fatih on 6/22/19.
//

#pragma once

#include <ble_types.h>
#include <common/ble/address.hpp>
#include <nrf_sdh.h>
#include <tos/uuid.hpp>

namespace tos::nrf52 {
enum class ble_errors
{
    invalid_addr = NRF_ERROR_INVALID_ADDR,
    invalid_param = NRF_ERROR_INVALID_PARAM,
    bad_size = NRF_ERROR_DATA_SIZE,
    forbidden = NRF_ERROR_FORBIDDEN
};

/**
 * NRF5 BLE SDK provides a way to store the large, 16 byte base UUIDs for GATT services
 * etc in the soft device memory, and access them through indices into this memory
 *
 * This type wraps such an index with an opaque interface
 */
class uuid_handle_t {
public:
    constexpr explicit uuid_handle_t(uint8_t hndl)
        : m_handle{hndl} {
    }

private:
    friend class overlay_uuid;
    uint8_t m_handle;
};

/**
 * NRF SDK provides a memory efficiency optimization for long UUIDs @ref uuid_handle_t
 *
 * The SDK also allows the 12th and 13th bytes of such UUIDs to be provided alongside the
 * handles to these UUIDs so that characteristics for GATT services could be handled
 * without allocating a new UUID
 *
 * This type provides an abstraction for that use case
 */
class overlay_uuid {
public:
    overlay_uuid() = default;

    explicit overlay_uuid(const uuid_handle_t& hndl) {
        m_uuid.type = hndl.m_handle;
    }

    void set_overlay(uint16_t words) {
        m_uuid.uuid = words;
    }

    ble_uuid_t& get() {
        return m_uuid;
    }

    const ble_uuid_t& get() const {
        return m_uuid;
    }

private:
    ble_uuid_t m_uuid;
};
} // namespace tos::nrf52
