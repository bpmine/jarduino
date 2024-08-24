import pymodbus
from pymodbus.client.sync import ModbusTcpClient as ModbusClient
import time

 
client = ModbusClient(host="192.168.3.203", port=502, auto_open=True, auto_close=True, timeout=10)

while True:
    try:
        data = client.read_coils(1, 2,unit=1)
        cmd=data.bits[1]
        print('Cmd: %d' % (cmd))

        data = client.read_discrete_inputs(1, 3,unit=1)
        print(data)
        N1=data.bits[0]
        N2=data.bits[1]
        N3=data.bits[2]
        print('N1= %d, N2=%d, N3=%d' % (N1,N2,N3))

        data = client.read_input_registers(1, 2,unit=1)
        RSSI=data.registers[0]
        PWR=data.registers[1]
        print('RSSI= %d, PWR= %d' % (RSSI,PWR) )
                
        client.write_coil(1,True,unit=1)

        client.write_coil(2,False,unit=1)
    except Exception as ex:
        print(ex)
        pass
    
    time.sleep(1)    


    
#data = client.read_holding_registers(100, unit=1)
#print(data.registers[0])

client.close()


