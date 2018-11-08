# Tos
[![CircleCI](https://circleci.com/gh/FatihBAKIR/tos.svg?style=svg&circle-token=acae0aae6ddbc486e9644319894828f403ae9e9f)](https://circleci.com/gh/FatihBAKIR/tos)

Tos is a cooperative multitasking library operating system focused on efficiency and compile time safety features.

Tos provides a modern programming environment on extremely resource constrained environments. It can work on boards with less than 1k of ram and 5k of program flash. These numbers are not just the OS, it also includes an application.

Supported targets:

| Arch   | CPU                 | Board                   |
|--------|---------------------|-------------------------|
| AVR    | atmega328p          | Uno, Nano, Pro Mini       |
| ARM    | cortex m3, cortex m4 | STM32F103, nRF528232     |
| Xtensa | lx106               | ESP8266 (any board) |
| x86    | any | User space of any OS |


## In a nutshell

+ Cooperative multitasking kernel
+ Efficient, zero overhead abstraction model
+ First class C++ support, with extensive standard library support, including C++2a features
+ Written using modern programming practices with focus on maintainability, safety and portability

## More

More info can be found at: [fatihbakir.net/tos](http://fatihbakir.net/tos)
