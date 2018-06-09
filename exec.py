#!/usr/bin/env python3

import os
import sys
import serial
import time

with serial.Serial(sys.argv[1], 9600) as ser:
    ser.write(bytes([ord('x'), int(sys.argv[2])]))
    print(ser.readline().decode("utf-8"))
