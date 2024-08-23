import serial
import serial.rs485
import time
import re
import threading

PORT='COM12'

def calcCS(buff):
    cs=0
    for b in range(1,len(buff)):
        #print('%d' % ord(buff[b]))
        cs=cs+ord(buff[b])

    return cs % 256

pResp=re.compile(r'.*\x01(.)(.)(.+)([0-9A-F]{2})\x02')
def decodeResp(txt):
    m=pResp.match(txt)
    if m!=None:
        addr=m.group(1)
        fct=m.group(2)
        datas=m.group(3)
        cs=m.group(4)

        return (addr,fct,datas)
    else:
        return ('','',txt)

    return None

def sendRequest(ser,addr,fct,val):
    cmd=''
    cmd+=chr(1)
    cmd+='%c' % addr
    cmd+='%c' % fct
    cmd+='%02X' % val
    cs=calcCS(cmd)
    cmd+='%02X' % cs
    cmd+=chr(2)

    req=bytes(cmd,'ascii')
    #print('Send %s' % (req))
    ser.write(req)
    resp=ser.read_until(chr(2))
    #print(resp)
    try:
        res=decodeResp(str(resp,'ascii'))
    except Exception as ex:
        res=None
        print(ex)
        
    if res!=None:
        (raddr,rfct,datas)=res
        if ((raddr==addr) and (rfct==fct)) or fct=='@':
            return datas
    
    return None


ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)
#ser.rs485_mode=serial.rs485.RS485Settings() #rts_level_for_tx=True, rts_level_for_rx=False, loopback=False) #, delay_before_tx=0.002f, delay_before_rx=0.001f)
time.sleep(2)
txt=ser.read_until('\n')
print(txt)
ser.flush()

def test_addr_fct(ser):
    print('_'*60)
    print('Test Addresse et Fonction..')
          
    res=sendRequest(ser,'A','@',ord('A'))
    print(res)
    assert res=='41'

    res=sendRequest(ser,'A','f',ord('1'))
    print(res)
    assert res=='31'

def cfg_ping(ser,addr):
    print('_'*60)
    print('PING %s' % (addr))
          
    res=sendRequest(ser,addr,'p',4)
    print(res)
    assert res=='%02x' % (4)

def cfg_addr(ser,oldAddr,addr):
    print('_'*60)
    print('Progr. Addr. %s -> %s' % (oldAddr,addr))
          
    res=sendRequest(ser,oldAddr,'@',ord(addr))
    print(res)
    assert res=='%02x' % (ord(addr))

def cfg_enable(ser,addr,en):
    print('_'*60)
    print('Enable Addr. %s: %d' % (addr,en))
          
    res=sendRequest(ser,addr,'e',en)
    print(res)
    assert res=='%02x' % (en)

def test_enable_disable(ser,addr):
    print('Test enable/disable')
    res=sendRequest(ser,addr,'e',1)
    print(res)
    assert(res)=='01'

    res=sendRequest(ser,addr,'1' if addr=='A' else '2',1)
    print(res)
    if addr=='A':
        assert(res)=='0C000081FF'
    else:
        assert(res)=='0E81FF'

    res=sendRequest(ser,addr,'1' if addr=='A' else '2',0)
    print(res)
    if addr=='A':
        assert(res)=='08000081FF'
    else:
        assert(res)=='0A81FF'
    
    res=sendRequest(ser,addr,'e',0)
    print(res)
    assert(res)=='00'

    res=sendRequest(ser,addr,'1' if addr=='A' else '2',1)
    print(res)
    if addr=='A':
        assert(res)=='04000081FF'
    else:
        assert(res)=='0681FF'

    res=sendRequest(ser,addr,'1' if addr=='A' else '2',0)
    print(res)
    if addr=='A':
        assert(res)=='00000081FF'
    else:
        assert(res)=='0281FF'


def test_full(ser,addr):
    res=sendRequest(ser,addr,'e',1)
    print(res)
    assert(res)=='01'

    res=sendRequest(ser,addr,'s',1)
    print('Compteur initial: %s' % res)
    time.sleep(5)
    for i in range(0,10):
        res=sendRequest(ser,addr,'1' if addr=='A' else '2',1)
        print(res)
        time.sleep(0.5)

    res=sendRequest(ser,addr,'1' if addr=='A' else '2',0)
    print(res)

    res=sendRequest(ser,addr,'s',1)
    print('Compteur final: %s' % (res))

    res=sendRequest(ser,addr,'S',1)
    print('RAZ: %s' % res)

    res=sendRequest(ser,addr,'s',1)
    print('Compteur: %s' % (res))

def test_basic_pump(ser):
    errs=0
    nbre=0
    
    res=sendRequest(ser,'A','S',1)
    print('RAZ: %s' % res)
    assert res=='00000000'

    res=sendRequest(ser,'A','e',1)
    print(res)
    assert(res)=='01'

    while True:
        nbre+=1
        res=sendRequest(ser,'A','1',0)
        
        if res==None:
            errs+=1
            print('Erreurs: %d/%d (%d%%)' % (errs,nbre,(errs*100/nbre)) )
        
        if nbre % 10 == 0:
            print(res)
            res=sendRequest(ser,'A','r',0)
            print(res)
            
        time.sleep(0.05)


def test_basic_oya(ser,addr):
    errs=0
    nbre=0
    
    res=sendRequest(ser,addr,'S',1)
    print('RAZ: %s' % res)
    assert res=='00000000'

    res=sendRequest(ser,addr,'e',1)
    print(res)
    assert(res)=='01'

    while True:
        nbre+=1
        res=sendRequest(ser,addr,'2',0)
        
        if res==None:
            errs+=1
            print('Erreurs: %d/%d (%d%%)' % (errs,nbre,(errs*100/nbre)) )
        
        if nbre % 10 == 0:
            print(res)
            res=sendRequest(ser,addr,'r',0)
            print(res)
            
        time.sleep(0.05)
        
   
time.sleep(2)
lst=['F']
while True:
    for a in lst:
        try:
            print('Ping %s: ' % (a),end='')
            res=sendRequest(ser,a,'p',4)
            print(res)
            assert res=='%02x' % (4)        
        except:
            print('NOK')
    
        time.sleep(0.01)
    
#cfg_addr(ser,'Z','B')        
#cfg_enable(ser,'B',1)

##time.sleep(2)
##for i in range(0,255+1):
##    print('addr: %s' % (chr(i)) )
##    res=sendRequest(ser,chr(i),'1',0)
##    print(res)
##    if res!=None:
##        time.sleep(2)

#test_addr_fct(ser)
#test_enable_disable(ser,'C')
#test_full(ser,'F')
#test_basic_pump(ser)
#test_basic_oya(ser,'D')

ser.close()
