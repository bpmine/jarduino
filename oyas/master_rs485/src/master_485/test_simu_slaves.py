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

def decodeInfo(msg):
    pCmdResp=re.compile(r'\x01([0-9A-F]{2})([po])([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{4})([0-9A-F]{0,4})([0-9A-F]{2})\x02')
    print('Recv> %s' % msg)
    txt=str(msg,'ascii')
    m=pCmdResp.match(txt)
    if m!=None:
        print('ok')
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

  
def sendInfo(ser,addr,inf):
    if addr==1:
        msg='\x011Dp%02X%02X%04X%02X%02X%02X%04X%04X%04X%02X\x02' % (
            addr,
            inf['status'],
            inf['tick'],
            inf['temp'],
            inf['hum'],
            inf['volt'],
            inf['time'],
            inf['errors'],
            inf['flow'],
            0
            )
    else:
        msg='\x0119o%02X%02X%04X%02X%02X%02X%04X%04X%02X\x02' % (
            addr,
            inf['status'],
            inf['tick'],
            inf['temp'],
            inf['hum'],
            inf['volt'],
            inf['time'],
            inf['errors'],
            0
            )

    calc_cs=calcCS(msg[3:-3])
    stcs='%02X' % (calc_cs)
    new_msg=msg[:-3]+stcs[0]+stcs[1]+msg[-1]
    msg=new_msg.encode('ascii')

    #print('Send> %s' % msg)
    
    #res=decodeInfo(msg)
    #print(res)
    if inf['status']&0x02:
        print(f'{addr} ENABLED')
        
    ser.write(msg)
            
    
def test_recv():
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

    while True:
        res=ser.read(100)
        if res!=b'':
            print(res)

    ser.close()
    

infos=[{
    'addr':i,
    'status':0,
    'tick':0,
    'temp':22,
    'hum':85,
    'volt':102,
    'time':0,
    'errors':0,
    'flow':0,
    'enabled':True
        } for i in range(0,8)]


def read_frame(ser):
    frame = bytearray()
    while True:
        byte = ser.read(1)
        if not byte:
            break
        if byte == b'\x01':
            frame = bytearray(b'\x01')
        elif frame:
            frame.append(byte[0])
            if byte == b'\x02':
                return frame

def decode_cmds(msg):    
    pDecCmds=re.compile(r'\x0109m([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})\x02')
    m=pDecCmds.match(msg)
    if m!=None:        
        cmds=int(m.group(1),16)
        addr=int(m.group(2),16)
        cs=int(m.group(3),16)
        calc_cs=calcCS(msg[3:-3])
        if calc_cs!=cs:
            print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
            return None

        return {
            'type':'cmds',
            'cmds':cmds,
            'addr':addr
            }
    else:
        return None


def decode_raz(msg):
    pDecRaz=re.compile(r'\x0105([a-z])([0-9A-F]{2})([0-9A-F]{2})\x02')
    m=pDecRaz.match(msg)
    if m!=None:                
        tp=m.group(1)
        addr=int(m.group(2),16)
        cs=int(m.group(3),16)
        calc_cs=calcCS(msg[3:-3])
        if calc_cs!=cs:
            print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
            return None

        if tp=='t':
            typ='raz_time'
        elif tp=='z':
            typ='raz_errs'
        else:
            return None
            
        return {
            'type':typ,
            'addr':addr            
            }
    else:
        return None


def decode_ping(msg):
    pDecRaz=re.compile(r'\x0107i([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})\x02')
    m=pDecRaz.match(msg)
    if m!=None:                
        addr=int(m.group(1),16)
        val=int(m.group(2),16)
        cs=int(m.group(3),16)
        calc_cs=calcCS(msg[3:-3])
        if calc_cs!=cs:
            print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
            return None
            
        return {
            'type':'ping',
            'addr':addr,
            'val':val
            }
    else:
        return None

    
def decode_frame(frame):    
    msg=str(frame,'ascii')
    res=decode_cmds(msg)
    if res!=None:
        return res

    res=decode_raz(msg)
    if res!=None:
        return res

    res=decode_ping(msg)
    if res!=None:
        return res

    return None


def treat_cmds(ser,obj):
    addr=obj['addr']
    cmds=obj['cmds']
    
    for at,inf in enumerate(infos):
        msk=1<<at
        if (cmds&msk)==msk:
            inf['status']|=0x3
        else:
            inf['status']&=~0x3

        if at==addr:
            sendInfo(ser,at,inf)

    if addr==ord('S'):
        print('SYNC')
        
        for i in infos:
            if i['enabled']==True:
                i['tick']+=1
                if i['status']&0x02==0x02:
                    i['time']+=1

            if i['time']>15:
                i['status']|=0x04;
            else:
                i['status']&=~0x04;
                
            if i['time']>90:
                i['status']|=0x08;
            else:
                i['status']&=~0x08;

            if i['addr']==2:
                if i['status']&0x04:
                    i['status']|=0x08;
                    
            if i['addr']==4:
                i['status']&=~0x04;
                i['status']&=~0x08;


def treat_raz(obj):
    addr=obj['addr']
    msg=obj['type']
    
    if addr<len(infos):                        
        if msg=='raz_time':
            print(f'Raz TIME @%1X' % (addr))
            infos[addr]['time']=0
        elif msg=='raz_errs':
            print(f'Raz ERR @%1X' % (addr))
            infos[addr]['errors']=0
        else:
            print(f'Raz ?')


def treat_ping(ser,obj):
    print(obj)
    addr=obj['addr']
    val=obj['val']
    
    msg='\x0107y%02X%02X%02X\x02' % (
            addr,
            val,
            0
            )

    calc_cs=calcCS(msg[3:-3])
    stcs='%02X' % (calc_cs)
    new_msg=msg[:-3]+stcs[0]+stcs[1]+msg[-1]
    msg=new_msg.encode('ascii')
        
    ser.write(msg)
    print('Send Pong: %s' % (msg))

    
def test_simu_slaves():
    print('Simulation des esclaves')
    ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

    time.sleep(2)
    res=ser.read(100)
    #print(res)

    #treat_ping(ser,{'addr':2,'val':0xAA})
    
    while True:
        frame=read_frame(ser)
        if frame:
            obj=decode_frame(frame)
            if obj==None:
                continue

            if obj['type']=='cmds':
                treat_cmds(ser,obj)
                continue
            elif obj['type'] in ['raz_time','raz_errs']:
                treat_raz(obj)
                continue
            elif obj['type']=='ping':
                time.sleep(0.05);
                treat_ping(ser,obj)
                continue


    ser.close()
    
#test_recv()
test_simu_slaves()

