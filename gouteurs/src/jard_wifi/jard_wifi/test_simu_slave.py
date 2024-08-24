import pymodbus
import serial
from pymodbus.pdu import ModbusRequest
from pymodbus.client.sync import ModbusSerialClient as ModbusClient #initialize a serial RTU client instance
from pymodbus.transaction import ModbusRtuFramer
import time

import logging
logging.basicConfig()
log = logging.getLogger()
#log.setLevel(logging.DEBUG)

def test_coils(cln):
    print('Read Coils')
    result= client.read_coils(0,13,unit= 4)
    print(result)
    for o in result.bits:
        print(o)

    print('Write Coils')
    result= client.write_coils(0,[True]*2,unit= 4)

    print('Read Coils')
    result= client.read_coils(0,2,unit= 4)
    for o in result.bits:
        print(o)



client= ModbusClient(method = "rtu", port=r"\\.\COM8",stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=2)

print('Connection:',end='')
res = client.connect()
if res==True:
    print('[OK]')
else:
    print('[NOK]')



test_coils(client)

client.close()
