#pragma once

#include <cstdint>

namespace tos::ebpf {
struct instruction {
    uint8_t opcode;
    uint8_t dst : 4;
    uint8_t src : 4;
    int16_t offset;
    int32_t imm;
};

constexpr auto CLS_MASK = 0x07;
constexpr auto ALU_OP_MASK = 0xf0;

constexpr auto CLS_LD = 0x00;   
constexpr auto CLS_LDX = 0x01;
constexpr auto CLS_ST = 0x02;
constexpr auto CLS_STX = 0x03;
constexpr auto CLS_ALU = 0x04;
constexpr auto CLS_JMP = 0x05;
constexpr auto CLS_ALU64 = 0x07;

constexpr auto SRC_IMM = 0x00;
constexpr auto SRC_REG = 0x08;

constexpr auto SIZE_W = 0x00;
constexpr auto SIZE_H = 0x08;
constexpr auto SIZE_B = 0x10;
constexpr auto SIZE_DW = 0x18;

/* Other memory modes are not yet supported */
constexpr auto MODE_IMM = 0x00;
constexpr auto MODE_MEM = 0x60;

constexpr auto OP_ADD_IMM = (CLS_ALU | SRC_IMM | 0x00);
constexpr auto OP_ADD_REG = (CLS_ALU | SRC_REG | 0x00);
constexpr auto OP_SUB_IMM = (CLS_ALU | SRC_IMM | 0x10);
constexpr auto OP_SUB_REG = (CLS_ALU | SRC_REG | 0x10);
constexpr auto OP_MUL_IMM = (CLS_ALU | SRC_IMM | 0x20);
constexpr auto OP_MUL_REG = (CLS_ALU | SRC_REG | 0x20);
constexpr auto OP_DIV_IMM = (CLS_ALU | SRC_IMM | 0x30);
constexpr auto OP_DIV_REG = (CLS_ALU | SRC_REG | 0x30);
constexpr auto OP_OR_IMM = (CLS_ALU | SRC_IMM | 0x40);
constexpr auto OP_OR_REG = (CLS_ALU | SRC_REG | 0x40);
constexpr auto OP_AND_IMM = (CLS_ALU | SRC_IMM | 0x50);
constexpr auto OP_AND_REG = (CLS_ALU | SRC_REG | 0x50);
constexpr auto OP_LSH_IMM = (CLS_ALU | SRC_IMM | 0x60);
constexpr auto OP_LSH_REG = (CLS_ALU | SRC_REG | 0x60);
constexpr auto OP_RSH_IMM = (CLS_ALU | SRC_IMM | 0x70);
constexpr auto OP_RSH_REG = (CLS_ALU | SRC_REG | 0x70);
constexpr auto OP_NEG = (CLS_ALU | 0x80);
constexpr auto OP_MOD_IMM = (CLS_ALU | SRC_IMM | 0x90);
constexpr auto OP_MOD_REG = (CLS_ALU | SRC_REG | 0x90);
constexpr auto OP_XOR_IMM = (CLS_ALU | SRC_IMM | 0xa0);
constexpr auto OP_XOR_REG = (CLS_ALU | SRC_REG | 0xa0);
constexpr auto OP_MOV_IMM = (CLS_ALU | SRC_IMM | 0xb0);
constexpr auto OP_MOV_REG = (CLS_ALU | SRC_REG | 0xb0);
constexpr auto OP_ARSH_IMM = (CLS_ALU | SRC_IMM | 0xc0);
constexpr auto OP_ARSH_REG = (CLS_ALU | SRC_REG | 0xc0);
constexpr auto OP_LE = (CLS_ALU | SRC_IMM | 0xd0);
constexpr auto OP_BE = (CLS_ALU | SRC_REG | 0xd0);

constexpr auto OP_ADD64_IMM = (CLS_ALU64 | SRC_IMM | 0x00);
constexpr auto OP_ADD64_REG = (CLS_ALU64 | SRC_REG | 0x00);
constexpr auto OP_SUB64_IMM = (CLS_ALU64 | SRC_IMM | 0x10);
constexpr auto OP_SUB64_REG = (CLS_ALU64 | SRC_REG | 0x10);
constexpr auto OP_MUL64_IMM = (CLS_ALU64 | SRC_IMM | 0x20);
constexpr auto OP_MUL64_REG = (CLS_ALU64 | SRC_REG | 0x20);
constexpr auto OP_DIV64_IMM = (CLS_ALU64 | SRC_IMM | 0x30);
constexpr auto OP_DIV64_REG = (CLS_ALU64 | SRC_REG | 0x30);
constexpr auto OP_OR64_IMM = (CLS_ALU64 | SRC_IMM | 0x40);
constexpr auto OP_OR64_REG = (CLS_ALU64 | SRC_REG | 0x40);
constexpr auto OP_AND64_IMM = (CLS_ALU64 | SRC_IMM | 0x50);
constexpr auto OP_AND64_REG = (CLS_ALU64 | SRC_REG | 0x50);
constexpr auto OP_LSH64_IMM = (CLS_ALU64 | SRC_IMM | 0x60);
constexpr auto OP_LSH64_REG = (CLS_ALU64 | SRC_REG | 0x60);
constexpr auto OP_RSH64_IMM = (CLS_ALU64 | SRC_IMM | 0x70);
constexpr auto OP_RSH64_REG = (CLS_ALU64 | SRC_REG | 0x70);
constexpr auto OP_NEG64 = (CLS_ALU64 | 0x80);
constexpr auto OP_MOD64_IMM = (CLS_ALU64 | SRC_IMM | 0x90);
constexpr auto OP_MOD64_REG = (CLS_ALU64 | SRC_REG | 0x90);
constexpr auto OP_XOR64_IMM = (CLS_ALU64 | SRC_IMM | 0xa0);
constexpr auto OP_XOR64_REG = (CLS_ALU64 | SRC_REG | 0xa0);
constexpr auto OP_MOV64_IMM = (CLS_ALU64 | SRC_IMM | 0xb0);
constexpr auto OP_MOV64_REG = (CLS_ALU64 | SRC_REG | 0xb0);
constexpr auto OP_ARSH64_IMM = (CLS_ALU64 | SRC_IMM | 0xc0);
constexpr auto OP_ARSH64_REG = (CLS_ALU64 | SRC_REG | 0xc0);

constexpr auto OP_LDXW = (CLS_LDX | MODE_MEM | SIZE_W);
constexpr auto OP_LDXH = (CLS_LDX | MODE_MEM | SIZE_H);
constexpr auto OP_LDXB = (CLS_LDX | MODE_MEM | SIZE_B);
constexpr auto OP_LDXDW = (CLS_LDX | MODE_MEM | SIZE_DW);
constexpr auto OP_STW = (CLS_ST | MODE_MEM | SIZE_W);
constexpr auto OP_STH = (CLS_ST | MODE_MEM | SIZE_H);
constexpr auto OP_STB = (CLS_ST | MODE_MEM | SIZE_B);
constexpr auto OP_STDW = (CLS_ST | MODE_MEM | SIZE_DW);
constexpr auto OP_STXW = (CLS_STX | MODE_MEM | SIZE_W);
constexpr auto OP_STXH = (CLS_STX | MODE_MEM | SIZE_H);
constexpr auto OP_STXB = (CLS_STX | MODE_MEM | SIZE_B);
constexpr auto OP_STXDW = (CLS_STX | MODE_MEM | SIZE_DW);
constexpr auto OP_LDDW = (CLS_LD | MODE_IMM | SIZE_DW);

constexpr auto OP_JA = (CLS_JMP | 0x00);
constexpr auto OP_JEQ_IMM = (CLS_JMP | SRC_IMM | 0x10);
constexpr auto OP_JEQ_REG = (CLS_JMP | SRC_REG | 0x10);
constexpr auto OP_JGT_IMM = (CLS_JMP | SRC_IMM | 0x20);
constexpr auto OP_JGT_REG = (CLS_JMP | SRC_REG | 0x20);
constexpr auto OP_JGE_IMM = (CLS_JMP | SRC_IMM | 0x30);
constexpr auto OP_JGE_REG = (CLS_JMP | SRC_REG | 0x30);
constexpr auto OP_JSET_REG = (CLS_JMP | SRC_REG | 0x40);
constexpr auto OP_JSET_IMM = (CLS_JMP | SRC_IMM | 0x40);
constexpr auto OP_JNE_IMM = (CLS_JMP | SRC_IMM | 0x50);
constexpr auto OP_JNE_REG = (CLS_JMP | SRC_REG | 0x50);
constexpr auto OP_JSGT_IMM = (CLS_JMP | SRC_IMM | 0x60);
constexpr auto OP_JSGT_REG = (CLS_JMP | SRC_REG | 0x60);
constexpr auto OP_JSGE_IMM = (CLS_JMP | SRC_IMM | 0x70);
constexpr auto OP_JSGE_REG = (CLS_JMP | SRC_REG | 0x70);
constexpr auto OP_CALL = (CLS_JMP | 0x80);
constexpr auto OP_EXIT = (CLS_JMP | 0x90);
constexpr auto OP_JLT_IMM = (CLS_JMP | SRC_IMM | 0xa0);
constexpr auto OP_JLT_REG = (CLS_JMP | SRC_REG | 0xa0);
constexpr auto OP_JLE_IMM = (CLS_JMP | SRC_IMM | 0xb0);
constexpr auto OP_JLE_REG = (CLS_JMP | SRC_REG | 0xb0);
constexpr auto OP_JSLT_IMM = (CLS_JMP | SRC_IMM | 0xc0);
constexpr auto OP_JSLT_REG = (CLS_JMP | SRC_REG | 0xc0);
constexpr auto OP_JSLE_IMM = (CLS_JMP | SRC_IMM | 0xd0);
constexpr auto OP_JSLE_REG = (CLS_JMP | SRC_REG | 0xd0);
} // namespace tos::ebpf