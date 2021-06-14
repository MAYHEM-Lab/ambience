# Simple VM

This is an example virtual machine written with `tvm`.

Instructions are defined in "instructions.hpp" and the vm state is in "vm_state.hpp".

## ISA

There are 16 16-bit registers in svm ISA. 15 of them are general purpose and one is
reserved for the instruction pointer.