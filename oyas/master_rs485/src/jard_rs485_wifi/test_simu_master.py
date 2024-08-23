import serial
import time
import re
import threading
import json

PORT='COM12'
SIMU=False

tick=0

def sendjs(ser,js):
    resp=bytes('\x01'+json.dumps(js)+'\x02','ascii')
    print('Send: %s' % resp)
    ser.write(resp)
   
def recv(ser):
    global tick
    
    tick+=1
    
    msg=ser.read_until(b'\x02')
    if msg!=b'':
        try:
            jsTxt=msg[1:-1]    
            js=json.loads(jsTxt)
            print('Doc: %s' % js)

            if js['req']=='cmds':
                print('cmds')
                ret={
                    'type':'cmds',
                    'cmds':js['cmds'],
                    'res':True,
                    'flow':0,
                    'ons':0,
                    'lows':0,
                    'highs':0,
                    'comms_ok':0,
                    'config_slaves':0
                    }
                sendjs(ser,ret)
                
            elif js['req']=='master':
                ret={
                    'type':'master',
                    'config_slaves':0,
                    'slaves':[]
                    }
                sendjs(ser,ret)
            elif js['req']=='pump':
                ret={
                    'type':'pump',
                    'addr':1,
                    'on':False,
                    'cmd':False,
                    'flow':0,
                    'temp':0,
                    'hum':0,
                    'voltage':0,
                    'comm_ok':True,
                    'tick':tick,
                    'total_on_s':4
                    }
                sendjs(ser,ret)
            elif js['req']=='oya':
                ret={
                    'type':'oya',
                    'addr':js['addr'],
                    'on':False,
                    'cmd':False,
                    'low':False,
                    'high':False,
                    'temp':0,
                    'hum':0,
                    'voltage':0,
                    'comm_ok':True,
                    'tick':tick,
                    'total_on_s':4
                    }
                sendjs(ser,ret)
        except Exception as ex:
            print(msg)
            print(ex)
            
            

def test():
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=2)
    time.sleep(0.1)
    res=ser.read(100)
    print(res)

    while True:
        recv(ser);
        time.sleep(0.2)

test()

