class: center, middle

# Tvm

A VM infrastructure for embedded systems

---

# Contents

+ Embedded systems primer & motivation
+ Goals and non-goals
+ Implementation
+ Results
+ Demo

---

# Embedded systems

General issues:

+ Can't program while running
+ Can't program wirelessly
+ No protection (MMU etc)
+ Lots of architectures: portability

---

# Embedded systems

VM Related issues:

+ Most optimizations are irrelevant
+ No cache, branch predictor or speculative execution
+ Can't execute from data space (Harvard Arch.)
+ Superinstructions are detrimental
+ Heavily application dependent

---

# Goals

+ Extremely little memory footprint
+ Zero overhead
+ Portable: can run on any arch
+ Compact byte code
+ Generic VM infrastructure
+ Easy to use
+ Standard C++, no extra build step

---

# Non Goals

+ Throughput.

---

# How Generic?

Extremely.

+ Can be stack based or register based
+ Can have 1 instruction or 1000
+ Optional heap
+ Optional stack
+ **Fully** custom ISA

---

# How Easy?

Quite.

Instructions are function objects:

```cpp
auto add = [](vm_state* st, reg_ind_t<4> reg, literal_t<16> l)
{
	st->registers[reg] += l;
};
```

VM state can be _anything_:
```cpp
struct vm_state
{
	std::array<uint16_t, 2> registers;
};
```

---

# What does it do then?

+ Execution
	+ Instruction decoding
	+ Instruction dispatch
+ Assembler
+ Disassembler

---

# ISA Description

```cpp
using my_ISA = tvm::list<
	ins<0x1, add>,
	ins<0x2, mul>,
	ins<0x3, push>,
	ins<0x4, pop>,
	ins<0x5, call>,
	ins<0x6, branch_if_eq>
>;
```

That's all!

---

# Instruction decoding

```cpp
auto add = [](vm_state* st, reg_ind_t<4> reg, literal_t<16> l)
{
	st->registers[reg] += l;
};

// later:
ins<0x1, add>
```

Tvm will inspect `add` to see operands:
0. N bits for opcode
1. `reg_ind_t<4>`: 4 bits register index
2. `literal_t<16>`: 16 bits literal

```
-----------------------------------------------
|                N + 20 Bits                  |
-----------------------------------------------
| N Bits |      4 Bits      |     16 Bits     |
-----------------------------------------------
| Opcode | Register Operand | Literal Operand |
-----------------------------------------------
```

---

# Execution

+ Tvm will generate a function at _compile time_ to decode and dispatch
each instruction.

+ These functions are placed in an array.

+ Executor reads N bits (opcode) from the stream

+ Indexes into the table

+ Function decodes and executes the instruction

+ Repeat

---

# Assembler

+ Variable length instructions

+ Difficult to write machine code

+ Need an assembler

+ Can't expect user to write their assembler

+ Must generate an assembler from the ISA description

---

# Usage

Provide names of instructions:

```cpp
auto add = [](vm_state* st, reg_ind_t<4> reg, literal_t<16> l)
{
	st->registers[reg] += l;
};

template<>  
struct instr_name<add>  
{  
    static constexpr auto value() { return "add"; }  
};
```

That's all!

---

# Assembler

+ Does type checking

+ Supports labels

Calculate sum of numbers up to 12:

```as
    movi    %r0, 12
    movi    %r1, 0
    movi    %r2, 0
loop:
    beq     %r0, %r2, done
    add     %r1, %r0
    movi    %r2, -1
    add     %r0, %r2
    movi    %r2, 0
    jump    loop
done:
    movr    %r0, %r1
    exit
```

---

# Where does it run?

Everywhere!

Completely standard C++14, uses no platform specific features.

If it runs C++14, it can run Tvm. Even at compile time!

---

# How much memory does it use?

---

# Demo

