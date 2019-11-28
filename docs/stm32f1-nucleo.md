# Getting Started on STM32F1 Nucleo Board

Install openocd and the gnu arm toolchain. See arm-common document for more.

Install cmake.

## Generate the project

In tos directory, create a build directory called `build-stm32f1` and generate the project for STM32F103RB MCU:

```
mkdir build-stm32f1
cd build-stm32f1
cmake -DTOS_CPU=stm32/f1/03rb -DCMAKE_BUILD_TYPE=MinSizeRel ..
```

Test your setup by building the core:

```
make tos_core
```

If this succeeds, you're ready to go! Go into the blink example directory in the build dir:

```
cd examples/stm32_blink
```

And build the example:

```
make blink
```

You can flash this image on the board using:

```
openocd -f target/stm32f1x.cfg -f interface/stlink.cfg -c "program blink verify reset exit"
```
