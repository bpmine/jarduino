import serial
import time
import re
import threading

PORT='COM15'
#PORT='COM21'
SIMU=False

def calcCS(buff):
    cs=0
    for b in range(0,len(buff)):
        #print('%d' % ord(buff[b]))
        cs=cs+ord(buff[b])

    return cs % 256


def decodePong(msg):
    pPong=re.compile(r'\x0107y([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})\x02')
    print('Recv> %s' % msg)
    txt=str(msg,'ascii')
    m=pPong.match(txt)
    if m!=None:
        addr=int(m.group(1),16)
        val=int(m.group(2),16)
        cs=int(m.group(3),16)

        calc_cs=calcCS(txt[3:-3])
        if (calc_cs!=cs):
            print('ERREUR: CS Incorrect %02X recu au lieu de %02X attendu' % (calc_cs,cs))
            return None

        return {'addr':'%02X' % (addr),'val':'%02X' % val}

    return None    

def sendPing(ser,addr,val):
    cmd=''
    cmd+=chr(1)
    cmd+="07"
    cmd+='i'
    cmd+='%02X' % (addr)
    cmd+='%02X' % (val)
    dta=cmd[3:]
    cs=calcCS(dta)
    cmd+='%02X' % cs
    cmd+=chr(2)

    req=bytes(cmd,'ascii')
    print('Send> %s' % (req))
    ser.write(req)
    msg=ser.read_until(b'\x02')
    pong=decodePong(msg)
    print(pong)

def sendRazT(ser,addr):
    cmd=''
    cmd+=chr(1)
    cmd+="05"
    cmd+='t'
    cmd+='%02X' % (addr)
    dta=cmd[3:]
    cs=calcCS(dta)
    cmd+='%02X' % cs
    cmd+=chr(2)

    req=bytes(cmd,'ascii')
    print('Send> %s' % (req))
    ser.write(req)
    msg=ser.read_until(b'\x02')
    pong=decodePong(msg)
    print(pong)
    
def decodeInfo(msg):
    pCmdResp=re.compile(r'\x01([0-9A-F]{2})([po])([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{4})([0-9A-F]{0,4})([0-9A-F]{2})\x02')
    print('Recv> %s' % msg)
    txt=str(msg,'ascii')
    m=pCmdResp.match(txt)
    if m!=None:
        size=int(m.group(1),16)
        typ=m.group(2)

        obj={
            'type':typ,
            'addr':m.group(3),
            'status':m.group(4),
            'tick':int(m.group(5),16),
            'temp':int(m.group(6),16),
            'hum':int(m.group(7),16),
            'volt':(float)(int(m.group(8),16)/10),
            'time':int(m.group(9),16),
            'errs':int(m.group(10),16)
            }

        if typ=='p':
            obj['flow']=int(m.group(11),16)
            cs=int(m.group(12),16)
        elif typ=='o':
            cs=int(m.group(12),16)
        else:
            print('ERROR: Incorrect type %s' % (typ))
            return None

        calc_cs=calcCS(txt[3:-3])

        if (calc_cs!=cs):
            print('ERREUR: CS Incorrect %02X recu au lieu de %02X attendu' % (calc_cs,cs))
            return None

        return obj

    return None
    
def sendCmd(ser,cmds,addr):
    cmd=''
    cmd+=chr(1)
    cmd+="09"
    cmd+='m'
    cmd+='%04X' % (cmds)
    cmd+='%02X' % (addr)
    dta=cmd[3:]
    cs=calcCS(dta)
    cmd+='%02X' % cs
    cmd+=chr(2)

    req=bytes(cmd,'ascii')
    print('Send %s' % (req))
    ser.write(req)
    msg=ser.read_until(b'\x02')
    if msg!=b'':
        obj=decodeInfo(msg)
        print(obj)

def test_recv():
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

    while True:
        res=ser.read(100)
        if res!=b'':
            print(res)

    ser.close()
    
def test_one_slave(addr):
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

    time.sleep(2)
    res=ser.read(100)
    print(res)


    sendPing(ser,2,254)
        
    for i in range(0,5):
        cmd=1<<addr
        sendCmd(ser,cmd,addr)    
        sendCmd(ser,cmd,ord('S'))
        time.sleep(1)

    for i in range(0,5):
        sendCmd(ser,0x0,addr)    
        sendCmd(ser,0x0,ord('S'))
        time.sleep(1)

    for i in range(0,10):
        sendCmd(ser,0x0,addr)
        time.sleep(1)

    sendRazT(ser,addr)
    
    sendCmd(ser,0x0,addr)
    sendCmd(ser,0x0,ord('S'))

    ser.close()

def test_endurance():
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

    time.sleep(2)
    res=ser.read(100)
    print(res)

    addr=6
    per=0
    on=True
    while True:
        per+=1
        if per>5:
            on=not on
            per=0

        msk_on=1<<6
        
        print('_'*60)
        print('Début cycle (%s):' % on)
        #sendCmd(ser,0x00,6)
        for i in range(1,7):
            sendCmd(ser,msk_on if on==True else 0x00,i)
            
        sendCmd(ser,msk_on if on==True else 0x00,ord('S'))
        time.sleep(2)


    ser.close()
    
#test_recv()
test_one_slave(6)
#test_endurance()
