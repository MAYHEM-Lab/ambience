#!/usr/bin/env python3

import os
import sys
import serial
import time

with serial.Serial(sys.argv[2], 19200) as ser:
    time.sleep(2)
    sz = os.stat(sys.argv[1]).st_size
    ser.write(bytes([ord('p'), sz]))
    print(ser.readline())
    print(ser.readline())

    with open(sys.argv[1], "rb") as prog:
        data = prog.read()
        ser.write(data)

    print(ser.readline())
