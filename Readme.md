# tVM

tvm is a lightweight and efficient byte code virtual machine 
infrastructure for resource constrained embedded systems 
applications.

---

tvm is quite heavy on the template metaprogramming side.
Thus, the parts require quite recent compilers. The runtime
can be compiled with C++14 compilers, however the assembler
and the disassembler require C++17 compilers.

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
