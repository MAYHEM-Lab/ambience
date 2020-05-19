# Tos
[![Build Status](http://drone.k3s.fatihbakir.net/api/badges/FatihBAKIR/tos/status.svg)](http://drone.k3s.fatihbakir.net/FatihBAKIR/tos)

Tos is a cooperative multitasking library operating system focused on efficiency and compile time safety features.

Tos provides a modern programming environment on extremely resource constrained environments. It can work on boards with less than 1k of ram and 5k of program flash. These numbers are not just the OS, it also includes an application.

Supported and tested targets:

| Arch | Controller |
|------|-----|
| AVR    | atmega328p |
| ARM    | STM32F103 (cortex m3) |
| ARM    | STM32L053 (cortex m0+) |
| ARM    | STM32L475 (cortex m4) |
| ARM    | STM32F746 (cortex m7) |
| ARM    | nRF528232 (cortex m4) |
| ARM    | nRF528240 (cortex m4) |
| ARM    | CC3235SF (cortex m4) |
| ARM    | CC3220SF (cortex m4) |
| Xtensa | ESP8266 |
| x86    | any (runs in userspace) |

## In a nutshell

+ Cooperative multitasking kernel, with optional per-thread preemption
+ Efficient, zero overhead abstraction model
+ First class C++ support, with extensive standard library support, including C++2a features
+ Written using modern programming practices with focus on maintainability, safety and portability

## More

More info can be found at: [fatihbakir.net/tos](http://fatihbakir.net/tos)
