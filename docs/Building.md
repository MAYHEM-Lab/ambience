# Building Tos

## AVR

You'll need:

+ `avr-gcc` toolchain (> 5.4)
+ `avr-libc`
+ `avrdude` to program the chips

```
mkdir avr_build
cd avr_build
cmake -DTOS_AVR=ON ..
cd examples/simple_portable
make
avr-objcopy -O ihex -R .eeprom simple_portable app.hex
avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:app.hex
```