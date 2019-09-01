//
// Created by fatih on 6/2/19.
//

#pragma once

#include "gatt.hpp"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"

#include <app_error.h>
#include <arch/ble/common.hpp>
#include <nrf_sdh.h>
#include <string_view>
#include <tos/expected.hpp>
#include <tos/ft.hpp>

namespace tos {
namespace nrf52 {
class softdev {
public:
    softdev();

    tos::expected<gatt, ble_errors> initialize_gatt();

    expected<uuid_handle_t, ble_errors> register_vs_uuid(const tos::uuid& uuid) {
        uint8_t hndl;
        static_assert(sizeof uuid == sizeof(ble_uuid128_t));
        // layouts of ble_uuid128_t and tos::uuid are the same, so this should work
        auto danger = reinterpret_cast<const ble_uuid128_t*>(&uuid);
        auto res = sd_ble_uuid_vs_add(danger, &hndl);
        if (res == NRF_SUCCESS)
            return uuid_handle_t{hndl};
        return unexpected(ble_errors(res));
    }

    ble_version_t get_version() const {
        ble_version_t vers;
        auto res = sd_ble_version_get(&vers);
        return vers;
    }

    ~softdev() {
        nrf_sdh_disable_request();
    }

private:
};
} // namespace nrf52
} // namespace tos