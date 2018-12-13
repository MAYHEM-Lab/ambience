//
// Created by fatih on 12/13/18.
//

#pragma once

#include <cstdint>
#include <array>

namespace tos
{
namespace rn2903
{
    struct dev_eui_t
    {
        std::array<uint8_t, 8> id;
    };

    struct app_eui_t
    {
        std::array<uint8_t, 8> id;
    };

    struct app_key_t
    {
        std::array<uint8_t, 16> id;
    };

    struct nvm_addr_t
    {
        uint16_t addr;
    };

    enum class failures
    {
        timeout,
        invalid_param,
        keys_not_init,
        no_free_ch,
        silent,
        busy,
        mac_paused
    };

    enum class join_res
    {
        joined,
        denied
    };
} // namespace rn2903
} // namespace tos