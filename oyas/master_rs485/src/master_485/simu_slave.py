import serial
import time
import re
import threading

SIMU=False

def calcCS(buff):
    cs=0
    for b in range(1,len(buff)):
        #print('%d' % ord(buff[b]))
        cs=cs+ord(buff[b])

    return cs % 256

pmp={
    'A':False
    }

oya={
    'B':False,
    'C':False,
    'D':False,
    'E':False,
    }

high={
    'B':True,
    'C':False,
    'D':False,
    'E':False,
    }

low={
    'B':True,
    'C':False,
    'D':False,
    'E':False,
    }

enable={
    'A':True,
    'B':True,
    'C':False,
    'D':False,
    'E':False,
    }

qty={
    'B':0,
    'C':0,
    'D':0,
    'E':0    
    }

PORT='COM26'
        
ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.05)
time.sleep(2)
txt=ser.read_until('\n')
#print(txt)
ser.flush()

pResp=re.compile(r'(.)(.)([0-9A-F]{2})([0-9A-F]{2})')
while True:
    b=ser.read()
    if b==b'\x01':
        resp=ser.read_until(b'\x02')

        try:
            txt=str(resp[:-1],'ascii')
        except:
            txt=''

        m=pResp.match(txt)
        if m!=None:
            addr=m.group(1)
            fct=m.group(2)
            cmd=m.group(3)
            cs=m.group(4)

            cs_calc=resp[0]+resp[1]+resp[2]+resp[3]
            cs_calc=cs_calc%256
            cs_calc='%02X' % cs_calc
            
            if cs_calc == cs:
                if fct=='1':                    
                    if addr in pmp:
                        pmp[addr]=True if cmd=='01' else False
                        
                        st=0
                        if pmp[addr]==True:
                            st=st|0x04
                        if enable[addr]==True:
                            st=st|0x08

                        ans='\x01%c%c%02X%04X%02X%02X' % (addr,fct,st,123,20,40)
                        cs=calcCS(ans)
                        ans+='%02X' % cs
                        ans+='\x02'
                        tosend=bytes(ans,'ascii')
                        print(tosend)

                        time.sleep(0.02)
                        ser.write(tosend)
                        ser.flush();
                    else:
                        print('Pump not found')
                elif fct=='2':
                    if addr in oya:                        
                        oya[addr]=True if cmd=='01' else False

                        st=0
                        if oya[addr]==True:
                            st=st|0x04
                        if enable[addr]==True:
                            st=st|0x08
                        if low[addr]==True:
                            st=st|0x01
                        if high[addr]==True:
                            st=st|0x02
                        
                        ans='\x01%c%c%02X%02X%02X' % (addr,fct,st,20,40)
                        cs=calcCS(ans)
                        ans+='%02X' % cs
                        ans+='\x02'
                        tosend=bytes(ans,'ascii')
                        print(tosend)

                        time.sleep(0.02)
                        ser.write(tosend)
                        ser.flush();

                        if oya[addr]==True:
                            if qty[addr]<20:
                                qty[addr]+=1
                        else:
                            if qty[addr]>0:
                                qty[addr]-=1

                        if qty[addr]>10:
                            low[addr]=False
                        else:
                            low[addr]=True

                        if qty[addr]>19:
                            high[addr]=False
                        else:
                            high[addr]=True
                                
                            
                    else:
                        print('Oya not found')


ser.close()





