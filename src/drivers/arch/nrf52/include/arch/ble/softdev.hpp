//
// Created by fatih on 6/2/19.
//

#pragma once

#include <nrf_sdh.h>
#include <tos/ft.hpp>
#include <tos/expected.hpp>
#include <app_error.h>
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include <string_view>
#include <arch/ble/common.hpp>

namespace tos
{
namespace nrf52
{
enum class softdev_errors
{
    invalid_addr = NRF_ERROR_INVALID_ADDR,
    invalid_param = NRF_ERROR_INVALID_PARAM,
    bad_size = NRF_ERROR_DATA_SIZE,
    forbidden = NRF_ERROR_FORBIDDEN
};

class softdev
{
public:
    softdev();

    expected<void, softdev_errors>
    set_device_name(std::string_view name);

    expected<void, softdev_errors>
    set_tx_power(int8_t power);

    expected<uuid_handle_t, softdev_errors>
    register_vs_uuid(const tos::uuid& uuid) {
        uint8_t hndl;
        static_assert(sizeof uuid == sizeof(ble_uuid128_t));
        // layouts of ble_uuid128_t and tos::uuid are the same, so this should work
        auto danger = reinterpret_cast<const ble_uuid128_t*>(&uuid);
        auto res = sd_ble_uuid_vs_add(danger, &hndl);
        if (res == NRF_SUCCESS) return uuid_handle_t{hndl};
        return unexpected(softdev_errors(res));
    }

    ble_version_t
    get_version() const {
        ble_version_t vers;
        auto res = sd_ble_version_get(&vers);
        return vers;
    }

    ~softdev() {
        nrf_sdh_disable_request();
    }
private:
};
}
}