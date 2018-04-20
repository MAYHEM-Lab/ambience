//
// Created by fatih on 4/20/18.
//

#include <tvm/instr_traits.hpp>
#include <tvm/tvm_types.hpp>
#include <tvm/decoding.hpp>
#include <tvm/instructions.hpp>

static_assert(decode_one<opcode_t, 1>(0x0FF).opcode == 0x7F);
static_assert(decode_one<opcode_t, 2>(0x0FF).opcode == 0x3F);

static_assert(std::is_same<offsets<list< opcode_t >>::type, std::index_sequence<0>>{});
static_assert(std::is_same<offsets<list< opcode_t, reg_ind_t<> >>::type, std::index_sequence<4, 0>>{});
static_assert(std::is_same<offsets<list< opcode_t, reg_ind_t<>, reg_ind_t<> >>::type, std::index_sequence<8, 4, 0>>{});

static_assert(decode(list<opcode_t>{}, 0xFF) == std::tuple<opcode_t>{ {0x7F} });
static_assert(decode(list<opcode_t, reg_ind_t<>>{}, 0xFF) == std::tuple<opcode_t, reg_ind_t<>>{ {0x0F}, {0x0F} });

static_assert(instruction_len<decltype(&add)>() == 2);
static_assert(instruction_len<decltype(&mov)>() == 4);
