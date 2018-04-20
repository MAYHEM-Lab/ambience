# tVM

tvm is a lightweight and efficient byte code virtual machine 
for resource constrained embedded systems applications.

---

tvm is quite heavy on the template metaprogramming side and
requires a C++17 compiler. Could be backported in the future.

---

## Building

```
mkdir build
cd build
cmake ..
make
```

## Subprojects

### ti

`ti` is the interpreter for tvm bytecode

### tdis

`tdis` is the disassembler for tvm bytecode

### tas

`tas` is the assembler for tvm bytecode

## ISA

`tvm` ISA supports variable length instructions up to 4 bytes.

+ `mov %register, $literal`: 
loads the given immediate value to the register

+ `add %reg1, %reg2`:
adds reg2 to reg1

## ABI

### Return values
Return values are returned via `%r0` register.

```
add(a, b):
    pop %r0
    pop %r1
    add %r0, %r1
    ret
```