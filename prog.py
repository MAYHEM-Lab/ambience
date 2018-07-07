#!/usr/bin/env python3

import os
import sys
import serial
import time
import socket

def program(src, part, ser):
    print(ser.readline())
    sz = os.stat(src).st_size
    ser.write(bytes([ord('p'), part, sz]))
    ser.flush()
    print(ser.readline())
    print(ser.readline())

    with open(src, "rb") as prog:
        data = prog.read()
        ser.write(data)
        ser.flush()

    print(ser.readline())

#with serial.Serial(sys.argv[2], 9600) as ser:
#    program(sys.argv[1], int(sys.argv[3], ser)


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((sys.argv[2], int(sys.argv[3])))

ser = s.makefile("rwb")

program(sys.argv[1], int(sys.argv[4]), ser)

