//
// Created by fatih on 12/8/19.
//

#pragma once

#include <crys_rnd.h>
#include <tos/debug/panic.hpp>
#include <utility>

namespace tos::nrf52::cc310 {
class rng {
public:
    rng() {
        auto res = CRYS_RndInit(&m_ctx, &m_workbuf);
        if (res != CRYS_OK) {
            tos::debug::panic("Can't init random number generator");
        }
    }

    auto operator()(tos::span<uint8_t> buf) {
        auto res = CRYS_RND_GenerateVector(&m_ctx, buf.size(), buf.data());
        if (res != CRYS_OK) {
            tos::debug::panic("Can't generate random vector");
        }
    }

    auto native_handle() {
        return std::make_pair(&m_ctx, &m_workbuf);
    }

private:
    CRYS_RND_State_t m_ctx;
    CRYS_RND_WorkBuff_t m_workbuf;
};
}