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

## ESP

You'll need:

+ `esp-open-sdk` in your home directory

```
mkdir esp_build
cd esp_build
cmake -DTOS_ESP=ON ..
cd examples/esp
make
export PATH=$PATH:~/esp-open-sdk/xtensa-lx106-elf/bin/
~/esp-open-sdk/esptool/esptool.py elf2image simple_esp
~/esp-open-sdk/esptool/esptool.py write_flash 0x00000 simple_esp-0x00000.bin 0x10000 simple_esp-0x10000.bin
```