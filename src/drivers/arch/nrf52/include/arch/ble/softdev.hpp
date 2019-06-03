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
    set_tx_power();
private:
};
}
}