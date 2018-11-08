# Tos
[![CircleCI](https://circleci.com/gh/FatihBAKIR/tos.svg?style=svg&circle-token=acae0aae6ddbc486e9644319894828f403ae9e9f)](https://circleci.com/gh/FatihBAKIR/tos)

Tos is a cooperative multitasking library operating system focused on efficiency and compile time safety features.

Tos provides a modern programming environment on extremely resource constrained environments. It can work on boards with less than 1k of ram and 5k of program flash. These numbers are not just the OS, it also includes an application.

Supported and tested targets:

| Arch | Controller |
|------|-----|
| AVR    | atmega328p |
| ARM    | STM32F103 (cortex m3) |
| ARM    | nRF528232 (cortex m4) |
| Xtensa | ESP8266 |
| ARM    | any (runs in userspace) |
| x86    | any (runs in userspace) |

Tos does not depend on the exact board the  as the IO is completely generic. It should run unmodified on any board as long as the controller is supported. For instance, it will work fine on your custom board with an atmega328p or esp8266.

## In a nutshell

+ Cooperative multitasking kernel
+ Efficient, zero overhead abstraction model
+ First class C++ support, with extensive standard library support, including C++2a features
+ Written using modern programming practices with focus on maintainability, safety and portability

## More

More info can be found at: [fatihbakir.net/tos](http://fatihbakir.net/tos)
