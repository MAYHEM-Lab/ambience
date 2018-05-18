//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#include <tvm/exec/executor.hpp>

#include "vm_def.hpp"
#include "vm_state.hpp"

#include <iostream>
#include <fstream>
#include <vector>

struct ptr_fetcher
{
    constexpr uint32_t fetch(uint16_t pc);
    uint8_t* m_base;
};

constexpr uint32_t ptr_fetcher::fetch(uint16_t pc)
{
    auto p = m_base + pc;
    uint32_t res = 0;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++;
    return res;
}

char tvm_stack[128];

int main(int argc, char** argv) {
    std::ifstream in(argv[1], std::ios::binary);
    std::vector<uint8_t> contents((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());

    ptr_fetcher fetch{contents.data()};
    tvm::vm_executor<ptr_fetcher, svm::vm_state, svm::ISA> exec(fetch);
    exec.m_state.stack_begin = (uint16_t*)tvm_stack;
    exec.m_state.stack_cur = (uint16_t*)tvm_stack;
    exec.m_state.stack_end = (uint16_t*)(tvm_stack + 128);
    exec.exec();

    std::cout << exec.m_state.registers[0] << '\n';

    return 0;
}