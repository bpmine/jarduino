#!/usr/bin/env python3
"""Pymodbus Synchronous Server Example.

The synchronous server is implemented in pure python without any third
party libraries (unless you need to use the serial protocols which require
pyserial). This is helpful in constrained or old environments where using
twisted is just not feasible. What follows is an example of its use:
"""
import logging

# --------------------------------------------------------------------------- #
# import the various server implementations
# --------------------------------------------------------------------------- #
from pymodbus.version import version

#from pymodbus.server.sync import StartTcpServer
# from pymodbus.server.sync import StartTlsServer
# from pymodbus.server.sync import StartUdpServer
from pymodbus.server.sync import StartSerialServer
from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock

# from pymodbus.datastore import ModbusSparseDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

from pymodbus.transaction import ModbusRtuFramer, ModbusBinaryFramer

# --------------------------------------------------------------------------- #
# configure the service logging
# --------------------------------------------------------------------------- #
FORMAT = (
    "%(asctime)-15s %(threadName)-15s"
    " %(levelname)-8s %(module)-15s:%(lineno)-8s %(message)s"
)
logging.basicConfig(format=FORMAT)
log = logging.getLogger()
#log.setLevel(logging.DEBUG)

mem_coils={}
for i in range(0,100):
    mem_coils[i]=False

mem_input_registers={}
for i in range(0,300):
    mem_input_registers[i]=0

mem_holding_registers={}
for i in range(0,300):
    mem_holding_registers[i]=0


#mem_coils[10]=True
#mem_coils[11]=True
#mem_coils[12]=True

mem_input_registers[200]=1
mem_input_registers[201]=1
mem_input_registers[202]=4

mem_input_registers[0]=120
mem_input_registers[1]=170
mem_input_registers[2]=27
mem_input_registers[3]=30

mem_holding_registers[0]=2022
mem_holding_registers[1]=7
mem_holding_registers[2]=17
mem_holding_registers[3]=18
mem_holding_registers[4]=45
mem_holding_registers[5]=5


def run_server():
    
    """Run server."""
    # ----------------------------------------------------------------------- #
    # initialize your data store
    # ----------------------------------------------------------------------- #
    # The datastores only respond to the addresses that they are initialized to
    # Therefore, if you initialize a DataBlock to addresses of 0x00 to 0xFF, a
    # request to 0x100 will respond with an invalid address exception. This is
    # because many devices exhibit this kind of behavior (but not all)::
    #
    #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
    #
    # Continuing, you can choose to use a sequential or a sparse DataBlock in
    # your data context.  The difference is that the sequential has no gaps in
    # the data while the sparse can. Once again, there are devices that exhibit
    # both forms of behavior::
    #
    #     block = ModbusSparseDataBlock({0x00: 0, 0x05: 1})
    #     block = ModbusSequentialDataBlock(0x00, [0]*5)
    #
    # Alternately, you can use the factory methods to initialize the DataBlocks
    # or simply do not pass them to have them initialized to 0x00 on the full
    # address range::
    #
    #     store = ModbusSlaveContext(di = ModbusSequentialDataBlock.create())
    #     store = ModbusSlaveContext()
    #
    # Finally, you are allowed to use the same DataBlock reference for every
    # table or you may use a separate DataBlock for each table.
    # This depends if you would like functions to be able to access and modify
    # the same data or not::
    #
    #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
    #     store = ModbusSlaveContext(di=block, co=block, hr=block, ir=block)
    #
    # The server then makes use of a server context that allows the server to
    # respond with different slave contexts for different unit ids. By default
    # it will return the same context for every unit id supplied (broadcast
    # mode).
    # However, this can be overloaded by setting the single flag to False and
    # then supplying a dictionary of unit id to context mapping::
    #
    #     slaves  = {
    #         0x01: ModbusSlaveContext(...),
    #         0x02: ModbusSlaveContext(...),
    #         0x03: ModbusSlaveContext(...),
    #     }
    #     context = ModbusServerContext(slaves=slaves, single=False)
    #
    # The slave context can also be initialized in zero_mode which means that a
    # request to address(0-7) will map to the address (0-7). The default is
    # False which is based on section 4.4 of the specification, so address(0-7)
    # will map to (1-8)::
    #
    #     store = ModbusSlaveContext(..., zero_mode=True)
    # ----------------------------------------------------------------------- #

    class MyDSCoils(ModbusSequentialDataBlock):
        def getValues(self,addr,count):
            print('READ COIL @%d -> %d' % (addr,count) )

            ret=[]
            for i in range(addr-1,addr+count-1):
                if i in mem_coils:
                    ret.append(mem_coils[i])
                else:
                    ret.append(False)

            print(ret)
                                
            return ret

        def setValues(self,addr,values):
            print('WRITE COIL @%d -> %s' % (addr,values) )

            for i in range(0,len(values)):
                if i+addr-1 in mem_coils:
                    mem_coils[addr+i-1]=values[i]


    class MyDSInpReg(ModbusSequentialDataBlock):
        def getValues(self,addr,count):
            print('INP REG @%d -> %d' % (addr,count) )

            ret=[]
            for i in range(addr-1,addr+count-1):
                if i in mem_input_registers:
                    ret.append(mem_input_registers[i])
                else:
                    ret.append(0)
                    
            print(ret)
                                
            return ret

    class MyDSHolReg(ModbusSequentialDataBlock):
        def getValues(self,addr,count):
            print('HOLDING REG @%d -> %d' % (addr,count) )

            ret=[]
            for i in range(addr-1,addr+count-1):
                if i in mem_holding_registers:
                    ret.append(mem_holding_registers[i])
                else:
                    ret.append(0)
                    
            print(ret)
                                
            return ret
        

    store = ModbusSlaveContext(
        di=ModbusSequentialDataBlock(0, [0xFFFF] * 100),
        co=MyDSCoils(0,[False] * 100),
        hr=MyDSHolReg(0, [0xFFFF] * 300),
        ir=MyDSInpReg(0, [0xFFFF] * 300),
    )

    context = ModbusServerContext(slaves=store, single=True)

    # ----------------------------------------------------------------------- #
    # initialize the server information
    # ----------------------------------------------------------------------- #
    # If you don"t set this or any fields, they are defaulted to empty strings.
    # ----------------------------------------------------------------------- #
    identity = ModbusDeviceIdentification()
    identity.VendorName="Pymodbus"
    identity.ProductCode="PM"
    identity.VendorUrl="http://github.com/riptideio/pymodbus/"
    identity.ProductName="Pymodbus Server"
    identity.ModelName="Pymodbus Server"
    identity.MajorMinorRevision= version.short()

    # ----------------------------------------------------------------------- #
    # run the server you want
    # ----------------------------------------------------------------------- #
    # Tcp:
    #StartTcpServer(context, identity=identity, address=("", 5020))
    #
    # TCP with different framer
    # StartTcpServer(context, identity=identity,
    #                framer=ModbusRtuFramer, address=("0.0.0.0", 5020))

    # TLS
    # StartTlsServer(context, identity=identity,
    #                certfile="server.crt", keyfile="server.key", password="pwd",
    #                address=("0.0.0.0", 8020))

    # Tls and force require client"s certificate for TLS full handshake:
    # StartTlsServer(context, identity=identity,
    #                certfile="server.crt", keyfile="server.key", password="pwd", reqclicert=True,
    #                address=("0.0.0.0", 8020))

    # Udp:
    # StartUdpServer(context, identity=identity, address=("0.0.0.0", 5020))

    # socat -d -d PTY,link=/tmp/ptyp0,raw,echo=0,ispeed=9600 PTY,
    #             link=/tmp/ttyp0,raw,echo=0,ospeed=9600
    # Ascii:
    # StartSerialServer(context, identity=identity,
    #                    port="/dev/ttyp0", timeout=1)

    # RTU:
    StartSerialServer(context, framer=ModbusRtuFramer, identity=identity,
                       port="COM7", timeout=.005, baudrate=9600)

    # Binary
    # StartSerialServer(context,
    #                   identity=identity,
    #                   framer=ModbusBinaryFramer,
    #                   port="/dev/ttyp0",
    #                   timeout=1)


if __name__ == "__main__":
    run_server()
