#!/usr/bin/env python3

import os
import sys
import serial
import time
import socket

def execute(part, ser):
    print(ser.readline())
    ser.write(bytes([ord('x'), part]))
    ser.flush()
    print(ser.readline().decode("utf-8"))


#with serial.Serial(sys.argv[1], 9600) as ser:
#    execute(int(sys.argv[2], ser)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((sys.argv[1], int(sys.argv[2])))

ser = s.makefile("rwb")

execute(int(sys.argv[3]), ser)

