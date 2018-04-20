# tVM

tvm is a lightweight and efficient byte code virtual machine 
for resource constrained embedded systems applications.

---

tVM requires a C++17 compiler.

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