# nRF52 DK Guide

## Pin info

### USART

+ RX Pin: 8
+ TX Pin: 6

## Install nrfjprog

Download and extract to an accessible location:
https://www.nordicsemi.com/?sc_itemid=%7B56868165-9553-444D-AA57-15BDE1BF6B49%7D

## Flashing a program

```sh
arm-none-eabi-objcopy -O ihex program program.hex
nrfjprog -f NRF52 --program program.hex --sectorerase -r --verify
```

+ Use **sectorerase** instead of **chiperase** for programs

## Flashing the softdevice

```sh
nrfjprog -f NRF52 --program \
nrf_sdk_root/components/softdevice/s140/hex/s140_nrf52_6.1.1_softdevice.hex \
--chiperase --verify
```

+ **chiperase** erases any user program, should program again.

## Attaching OpenOCD for debugging

```sh
openocd_root/src/openocd -s openocd_root/tcl/ -f board/nordic_nrf52_dk.cfg
```