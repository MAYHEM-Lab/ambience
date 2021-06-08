//
// Created by fatih on 6/2/21.
//

#include <tos/ebpf/opcode.hpp>
#include <tos/ebpf/verify.hpp>

namespace tos::ebpf {
namespace {
constexpr auto max_instrs = 4096;
constexpr auto max_ext_funcs = 16;

const char* ubpf_error(const char* fmt, ...) {
    return fmt;
}
} // namespace
bool validate(span<const instruction> instrs, const char** errmsg) {
    if (instrs.size() >= max_instrs) {
        *errmsg = "too many instructions";
        return false;
    }

    int i;
    for (i = 0; i < instrs.size(); i++) {
        auto& inst = instrs[i];
        bool store = false;

        switch (inst.opcode) {
        case OP_ADD_IMM:
        case OP_ADD_REG:
        case OP_SUB_IMM:
        case OP_SUB_REG:
        case OP_MUL_IMM:
        case OP_MUL_REG:
        case OP_DIV_REG:
        case OP_OR_IMM:
        case OP_OR_REG:
        case OP_AND_IMM:
        case OP_AND_REG:
        case OP_LSH_IMM:
        case OP_LSH_REG:
        case OP_RSH_IMM:
        case OP_RSH_REG:
        case OP_NEG:
        case OP_MOD_REG:
        case OP_XOR_IMM:
        case OP_XOR_REG:
        case OP_MOV_IMM:
        case OP_MOV_REG:
        case OP_ARSH_IMM:
        case OP_ARSH_REG:
            break;

        case OP_LE:
        case OP_BE:
            if (inst.imm != 16 && inst.imm != 32 && inst.imm != 64) {
                *errmsg = ubpf_error("invalid endian immediate at PC %d", i);
                return false;
            }
            break;

        case OP_ADD64_IMM:
        case OP_ADD64_REG:
        case OP_SUB64_IMM:
        case OP_SUB64_REG:
        case OP_MUL64_IMM:
        case OP_MUL64_REG:
        case OP_DIV64_REG:
        case OP_OR64_IMM:
        case OP_OR64_REG:
        case OP_AND64_IMM:
        case OP_AND64_REG:
        case OP_LSH64_IMM:
        case OP_LSH64_REG:
        case OP_RSH64_IMM:
        case OP_RSH64_REG:
        case OP_NEG64:
        case OP_MOD64_REG:
        case OP_XOR64_IMM:
        case OP_XOR64_REG:
        case OP_MOV64_IMM:
        case OP_MOV64_REG:
        case OP_ARSH64_IMM:
        case OP_ARSH64_REG:
            break;

        case OP_LDXW:
        case OP_LDXH:
        case OP_LDXB:
        case OP_LDXDW:
            break;

        case OP_STW:
        case OP_STH:
        case OP_STB:
        case OP_STDW:
        case OP_STXW:
        case OP_STXH:
        case OP_STXB:
        case OP_STXDW:
            store = true;
            break;

        case OP_LDDW:
            if (i + 1 >= instrs.size() || instrs[i + 1].opcode != 0) {
                *errmsg = ubpf_error("incomplete lddw at PC %d", i);
                return false;
            }
            i++; /* Skip next instruction */
            break;

        case OP_JA:
        case OP_JEQ_REG:
        case OP_JEQ_IMM:
        case OP_JGT_REG:
        case OP_JGT_IMM:
        case OP_JGE_REG:
        case OP_JGE_IMM:
        case OP_JLT_REG:
        case OP_JLT_IMM:
        case OP_JLE_REG:
        case OP_JLE_IMM:
        case OP_JSET_REG:
        case OP_JSET_IMM:
        case OP_JNE_REG:
        case OP_JNE_IMM:
        case OP_JSGT_IMM:
        case OP_JSGT_REG:
        case OP_JSGE_IMM:
        case OP_JSGE_REG:
        case OP_JSLT_IMM:
        case OP_JSLT_REG:
        case OP_JSLE_IMM:
        case OP_JSLE_REG: {
            if (inst.offset == -1) {
                *errmsg = ubpf_error("infinite loop at PC %d", i);
                return false;
            }
            int new_pc = i + 1 + inst.offset;
            if (new_pc < 0 || new_pc >= instrs.size()) {
                *errmsg = ubpf_error("jump out of bounds at PC %d", i);
                return false;
            } else if (instrs[new_pc].opcode == 0) {
                *errmsg = ubpf_error("jump to middle of lddw at PC %d", i);
                return false;
            }
            break;
        }

        case OP_CALL:
            if (inst.imm < 0 || inst.imm >= max_ext_funcs) {
                *errmsg = ubpf_error("invalid call immediate at PC %d", i);
                return false;
            }
            break;

        case OP_EXIT:
            break;

        case OP_DIV_IMM:
        case OP_MOD_IMM:
        case OP_DIV64_IMM:
        case OP_MOD64_IMM:
            if (inst.imm == 0) {
                *errmsg = ubpf_error("division by zero at PC %d", i);
                return false;
            }
            break;

        default:
            *errmsg = ubpf_error("unknown opcode 0x%02x at PC %d", inst.opcode, i);
            return false;
        }

        if (inst.src > 10) {
            *errmsg = ubpf_error("invalid source register at PC %d", i);
            return false;
        }

        if (inst.dst > 9 && !(store && inst.dst == 10)) {
            *errmsg = ubpf_error("invalid destination register at PC %d", i);
            return false;
        }
    }

    return true;
}
} // namespace tos::ebpf
