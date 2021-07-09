#pragma once

#include <tos/board.hpp>
#include <common/common_timer_multiplexer.hpp>

struct platform_group_args {};

class platform_support {
public:
    void stage1_init() {
    }

    void stage2_init();

    auto init_serial() {
        return tos::bsp::board_spec::default_com::open();
    }

    auto& get_chrono() {
        return m_chrono;
    }

    tos::common_timer_multiplex<tos::hosted::timer> m_chrono;

    platform_group_args make_args(){
        return {};
    }
};