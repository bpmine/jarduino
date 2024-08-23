import serial
import time
import re
import threading

PORT='COM10'

ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=10)

while True:
    txt=ser.read_until(b'\x02')
    if txt!=b'':
        print(txt)

ser.close()
