//
// Created by fatih on 5/7/18.
//

#pragma once

#include <stdint.h>

namespace tos {
struct csd_v1_t {
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned c_size_high : 2;
    unsigned reserved2 : 2;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign : 1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    uint8_t c_size_mid;
    // byte 8
    unsigned vdd_r_curr_max : 3;
    unsigned vdd_r_curr_min : 3;
    unsigned c_size_low : 2;
    // byte 9
    unsigned c_size_mult_high : 2;
    unsigned vdd_w_cur_max : 3;
    unsigned vdd_w_curr_min : 3;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned c_size_mult_low : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved3 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved4 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved5 : 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
};

struct csd_v2_t {
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned reserved2 : 4;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign : 1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    unsigned reserved3 : 2;
    unsigned c_size_high : 6;
    // byte 8
    uint8_t c_size_mid;
    // byte 9
    uint8_t c_size_low;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned reserved4 : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved5 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved6 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved7 : 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
};

union csd_t {
    csd_v1_t v1;
    csd_v2_t v2;
};
} // namespace tos