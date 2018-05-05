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

## ISA

tvm is a VM infrastructure and doesn't directly define an ISA or an
ABI for anything. Users can define and implement their own ISA on top
of what tvm provides.

Implementing instructions is quite simple. Every instruction is implemented
in terms of a C++ function object type. Since C++14 doesn't support `auto` template
arguments, free standing functions do not work.

An example instruction implementation looks like this:

```cpp
// add %r0, %r1 // adds r1 to r0

struct add
{
    constexpr void operator()(vm_state* state, reg_ind_t<4> to, reg_ind_t<4> from)
    {
        state->registers[to.ind] += state->registers[from.ind];
    }
};

/**
 * This is required for the assembler and the disassembler and is not needed
 * for the runtime.
 */
template<> struct instr_name<add> { static constexpr auto value() { return "add"; } }
```