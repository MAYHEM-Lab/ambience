#include <cstdint>
#include <tos/debug/log.hpp>
#include <tos/ebpf/vm.hpp>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobe16(x) (x)
#define htole16(x) (x)
#define be16toh(x) (x)
#define le16toh(x) (x)
#define htobe32(x) (x)
#define htole32(x) (x)
#define be32toh(x) (x)
#define le32toh(x) (x)
#define htobe64(x) (x)
#define htole64(x) (x)
#define be64toh(x) (x)
#define le64toh(x) (x)
#else
#define htobe16(x) __uint16_identity(x)
#define htole16(x) __bswap_16(x)
#define be16toh(x) __uint16_identity(x)
#define le16toh(x) __bswap_16(x)
#define htobe32(x) __uint32_identity(x)
#define htole32(x) __bswap_32(x)
#define be32toh(x) __uint32_identity(x)
#define le32toh(x) __bswap_32(x)
#define htobe64(x) __uint64_identity(x)
#define htole64(x) __bswap_64(x)
#define be64toh(x) __uint64_identity(x)
#define le64toh(x) __bswap_64(x)
#endif

namespace tos::ebpf {
static bool bounds_check(void* addr,
                         int size,
                         const char* type,
                         uint16_t cur_pc,
                         const void* mem,
                         size_t mem_len,
                         void* stack);
static constexpr auto stack_size = 512;
uint64_t execute(const execution_context& ctx, span<uint64_t> args) {
    std::array<uint64_t, 16> reg;
    std::array<uint64_t, (stack_size + 7) / 8> stack;

    //    reg[1] = reinterpret_cast<uintptr_t>(ctx.memory.data());
    //    reg[2] = static_cast<uint64_t>(ctx.memory.size());
    reg[10] = reinterpret_cast<uintptr_t>(stack.end());

    for (int i = 0; i < std::min<int>(args.size(), 5); ++i) {
        reg[1 + i] = args[i];
    }

    uint16_t pc = 0;

    while (true) {
        const uint16_t cur_pc = pc;
        auto inst = ctx.instructions[pc++];

        switch (inst.opcode) {
        case OP_ADD_IMM:
            reg[inst.dst] += inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_ADD_REG:
            reg[inst.dst] += reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_SUB_IMM:
            reg[inst.dst] -= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_SUB_REG:
            reg[inst.dst] -= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MUL_IMM:
            reg[inst.dst] *= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MUL_REG:
            reg[inst.dst] *= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;

        case OP_DIV_IMM:
            reg[inst.dst] = uint32_t(reg[inst.dst]) / uint32_t(inst.imm);
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_DIV_REG:
            if (reg[inst.src] == 0) {
                //                LOG_ERROR("division by zero at PC", cur_pc);
                return UINT64_MAX;
            }
            reg[inst.dst] = uint32_t(reg[inst.dst]) / uint32_t(reg[inst.src]);
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_OR_IMM:
            reg[inst.dst] |= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_OR_REG:
            reg[inst.dst] |= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_AND_IMM:
            reg[inst.dst] &= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_AND_REG:
            reg[inst.dst] &= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_LSH_IMM:
            reg[inst.dst] <<= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_LSH_REG:
            reg[inst.dst] <<= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_RSH_IMM:
            reg[inst.dst] = uint32_t(reg[inst.dst]) >> inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_RSH_REG:
            reg[inst.dst] = uint32_t(reg[inst.dst]) >> reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_NEG:
            reg[inst.dst] = -(int64_t)reg[inst.dst];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MOD_IMM:
            reg[inst.dst] = uint32_t(reg[inst.dst]) % uint32_t(inst.imm);
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MOD_REG:
            if (reg[inst.src] == 0) {
                //                LOG_ERROR("Division by zero at PC", cur_pc);
                return UINT64_MAX;
            }
            reg[inst.dst] = uint32_t(reg[inst.dst]) % uint32_t(reg[inst.src]);
            break;
        case OP_XOR_IMM:
            reg[inst.dst] ^= inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_XOR_REG:
            reg[inst.dst] ^= reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MOV_IMM:
            reg[inst.dst] = inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_MOV_REG:
            reg[inst.dst] = reg[inst.src];
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_ARSH_IMM:
            reg[inst.dst] = (int32_t)reg[inst.dst] >> inst.imm;
            reg[inst.dst] &= UINT32_MAX;
            break;
        case OP_ARSH_REG:
            reg[inst.dst] = (int32_t)reg[inst.dst] >> uint32_t(reg[inst.src]);
            reg[inst.dst] &= UINT32_MAX;
            break;

        case OP_LE:
            if (inst.imm == 16) {
                reg[inst.dst] = htole16(reg[inst.dst]);
            } else if (inst.imm == 32) {
                reg[inst.dst] = htole32(reg[inst.dst]);
            } else if (inst.imm == 64) {
                reg[inst.dst] = htole64(reg[inst.dst]);
            }
            break;
        case OP_BE:
            if (inst.imm == 16) {
                reg[inst.dst] = htobe16(reg[inst.dst]);
            } else if (inst.imm == 32) {
                reg[inst.dst] = htobe32(reg[inst.dst]);
            } else if (inst.imm == 64) {
                reg[inst.dst] = htobe64(reg[inst.dst]);
            }
            break;


        case OP_ADD64_IMM:
            reg[inst.dst] += inst.imm;
            break;
        case OP_ADD64_REG:
            reg[inst.dst] += reg[inst.src];
            break;
        case OP_SUB64_IMM:
            reg[inst.dst] -= inst.imm;
            break;
        case OP_SUB64_REG:
            reg[inst.dst] -= reg[inst.src];
            break;
        case OP_MUL64_IMM:
            reg[inst.dst] *= inst.imm;
            break;
        case OP_MUL64_REG:
            reg[inst.dst] *= reg[inst.src];
            break;
        case OP_DIV64_IMM:
            reg[inst.dst] /= inst.imm;
            break;
        case OP_DIV64_REG:
            if (reg[inst.src] == 0) {
                //                LOG_ERROR("Division by zero at PC", cur_pc);
                return UINT64_MAX;
            }
            reg[inst.dst] /= reg[inst.src];
            break;
        case OP_OR64_IMM:
            reg[inst.dst] |= inst.imm;
            break;
        case OP_OR64_REG:
            reg[inst.dst] |= reg[inst.src];
            break;
        case OP_AND64_IMM:
            reg[inst.dst] &= inst.imm;
            break;
        case OP_AND64_REG:
            reg[inst.dst] &= reg[inst.src];
            break;
        case OP_LSH64_IMM:
            reg[inst.dst] <<= inst.imm;
            break;
        case OP_LSH64_REG:
            reg[inst.dst] <<= reg[inst.src];
            break;
        case OP_RSH64_IMM:
            reg[inst.dst] >>= inst.imm;
            break;
        case OP_RSH64_REG:
            reg[inst.dst] >>= reg[inst.src];
            break;
        case OP_NEG64:
            reg[inst.dst] = -reg[inst.dst];
            break;
        case OP_MOD64_IMM:
            reg[inst.dst] %= inst.imm;
            break;
        case OP_MOD64_REG:
            if (reg[inst.src] == 0) {
                //                LOG_ERROR("Division by zero at PC", cur_pc);
                return UINT64_MAX;
            }
            reg[inst.dst] %= reg[inst.src];
            break;
        case OP_XOR64_IMM:
            reg[inst.dst] ^= inst.imm;
            break;
        case OP_XOR64_REG:
            reg[inst.dst] ^= reg[inst.src];
            break;
        case OP_MOV64_IMM:
            reg[inst.dst] = inst.imm;
            break;
        case OP_MOV64_REG:
            reg[inst.dst] = reg[inst.src];
            break;
        case OP_ARSH64_IMM:
            reg[inst.dst] = (int64_t)reg[inst.dst] >> inst.imm;
            break;
        case OP_ARSH64_REG:
            reg[inst.dst] = (int64_t)reg[inst.dst] >> reg[inst.src];
            break;

#define BOUNDS_CHECK_LOAD(sz)                                 \
    do {                                                      \
        if (!bounds_check((char*)reg[inst.src] + inst.offset, \
                          sz,                                 \
                          "load",                             \
                          cur_pc,                             \
                          ctx.memory.data(),                  \
                          ctx.memory.size(),                  \
                          stack.data())) {                    \
            return UINT64_MAX;                                \
        }                                                     \
    } while (0)
#define BOUNDS_CHECK_STORE(sz)                                \
    do {                                                      \
        if (!bounds_check((char*)reg[inst.dst] + inst.offset, \
                          sz,                                 \
                          "store",                            \
                          cur_pc,                             \
                          ctx.memory.data(),                  \
                          ctx.memory.size(),                  \
                          stack.data())) {                    \
            return UINT64_MAX;                                \
        }                                                     \
    } while (0)

        case OP_LDXW:
            BOUNDS_CHECK_LOAD(4);
            reg[inst.dst] = *(uint32_t*)(uintptr_t)(reg[inst.src] + inst.offset);
            break;
        case OP_LDXH:
            BOUNDS_CHECK_LOAD(2);
            reg[inst.dst] = *(uint16_t*)(uintptr_t)(reg[inst.src] + inst.offset);
            break;
        case OP_LDXB:
            BOUNDS_CHECK_LOAD(1);
            reg[inst.dst] = *(uint8_t*)(uintptr_t)(reg[inst.src] + inst.offset);
            break;
        case OP_LDXDW:
            BOUNDS_CHECK_LOAD(8);
            reg[inst.dst] = *(uint64_t*)(uintptr_t)(reg[inst.src] + inst.offset);
            break;

        case OP_STW:
            BOUNDS_CHECK_STORE(4);
            *(uint32_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = inst.imm;
            break;
        case OP_STH:
            BOUNDS_CHECK_STORE(2);
            *(uint16_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = inst.imm;
            break;
        case OP_STB:
            BOUNDS_CHECK_STORE(1);
            *(uint8_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = inst.imm;
            break;
        case OP_STDW:
            BOUNDS_CHECK_STORE(8);
            *(uint64_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = inst.imm;
            break;

        case OP_STXW:
            BOUNDS_CHECK_STORE(4);
            *(uint32_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = reg[inst.src];
            break;
        case OP_STXH:
            BOUNDS_CHECK_STORE(2);
            *(uint16_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = reg[inst.src];
            break;
        case OP_STXB:
            BOUNDS_CHECK_STORE(1);
            *(uint8_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = reg[inst.src];
            break;
        case OP_STXDW:
            BOUNDS_CHECK_STORE(8);
            *(uint64_t*)(uintptr_t)(reg[inst.dst] + inst.offset) = reg[inst.src];
            break;

        case OP_LDDW:
            reg[inst.dst] =
                (uint32_t)inst.imm | ((uint64_t)ctx.instructions[pc++].imm << 32);
            break;

        case OP_JA:
            pc += inst.offset;
            break;
        case OP_JEQ_IMM:
            if (reg[inst.dst] == static_cast<uint64_t>(inst.imm)) {
                pc += inst.offset;
            }
            break;
        case OP_JEQ_REG:
            if (reg[inst.dst] == reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JGT_IMM:
            if (reg[inst.dst] > (uint32_t)inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JGT_REG:
            if (reg[inst.dst] > reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JGE_IMM:
            if (reg[inst.dst] >= (uint32_t)inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JGE_REG:
            if (reg[inst.dst] >= reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JLT_IMM:
            if (reg[inst.dst] < (uint32_t)inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JLT_REG:
            if (reg[inst.dst] < reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JLE_IMM:
            if (reg[inst.dst] <= (uint32_t)inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JLE_REG:
            if (reg[inst.dst] <= reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JSET_IMM:
            if (reg[inst.dst] & inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JSET_REG:
            if (reg[inst.dst] & reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JNE_IMM:
            if (reg[inst.dst] != static_cast<uint64_t>(inst.imm)) {
                pc += inst.offset;
            }
            break;
        case OP_JNE_REG:
            if (reg[inst.dst] != reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JSGT_IMM:
            if ((int64_t)reg[inst.dst] > inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JSGT_REG:
            if ((int64_t)reg[inst.dst] > (int64_t)reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JSGE_IMM:
            if ((int64_t)reg[inst.dst] >= inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JSGE_REG:
            if ((int64_t)reg[inst.dst] >= (int64_t)reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JSLT_IMM:
            if ((int64_t)reg[inst.dst] < inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JSLT_REG:
            if ((int64_t)reg[inst.dst] < (int64_t)reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_JSLE_IMM:
            if ((int64_t)reg[inst.dst] <= inst.imm) {
                pc += inst.offset;
            }
            break;
        case OP_JSLE_REG:
            if ((int64_t)reg[inst.dst] <= (int64_t)reg[inst.src]) {
                pc += inst.offset;
            }
            break;
        case OP_EXIT:
            return reg[0];
        case OP_CALL:
            reg[0] = ctx.ext_funcs[inst.imm](reg[1], reg[2], reg[3], reg[4], reg[5]);
            break;
        }
    }
}

static bool bounds_check(void* addr,
                         int size,
                         const char* type,
                         uint16_t cur_pc,
                         const void* mem,
                         size_t mem_len,
                         void* stack) {
    if (mem && (addr >= mem && ((char*)addr + size) <= ((char*)mem + mem_len))) {
        /* Context access */
        return true;
    } else if (addr >= stack && ((char*)addr + size) <= ((char*)stack + stack_size)) {
        /* Stack access */
        return true;
    } else {
        LOG_ERROR("uBPF error: out of bounds memory %s at PC %u, addr %p, size %d\nmem "
                  "%p/%zd stack %p/%d\n",
                  type,
                  cur_pc,
                  addr,
                  size,
                  mem,
                  mem_len,
                  stack,
                  stack_size);
        return false;
    }
}
} // namespace tos::ebpf