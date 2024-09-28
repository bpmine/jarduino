import pymodbus

TMT=5
SIMU=False

try:
    from pymodbus.client import ModbusTcpClient as ModbusClient
except:
    try:
        from pymodbus.client.sync import ModbusTcpClient as ModbusClient
    except:
        pass
    
import time
from threading import Thread
import redis

class WifiIoMgr:
    def __init__(self,name,ip,port=502):
        self.name=name
        self.ip=ip
        self.port=port

        self.cmd=False
        self.N1=False
        self.N2=False
        self.N3=False
        self.RSSI=0
        self.PWR=0
        self.valid=False
        
        self.t = Thread(target=self.loop, args=[])
        self.t.start()

    def loop(self):
        if SIMU==False:
            print('Start tache de fond %s...' % (self.name) )        
            cln = ModbusClient(host=self.ip, port=self.port, auto_open=True, auto_close=True, timeout=10)
            while True:
                try:
                    data = cln.read_coils(1, 2,unit=1)
                    cmd=data.bits[1]

                    data = cln.read_discrete_inputs(1, 3,unit=1)
                    self.N1=data.bits[0]
                    self.N2=data.bits[1]
                    self.N3=data.bits[2]

                    data = cln.read_input_registers(1, 2,unit=1)
                    self.RSSI=data.registers[0]
                    self.PWR=data.registers[1]

                    cln.write_coils(1,[True,self.cmd],unit=1)
                    
                    self.valid=True

                except Exception as ex:
                    print('%s: %s' % (self.name,ex))
                    self.valid=False

                time.sleep(0.4)
                
            cln.close()
        else:
            print('Start tache de fond %s en MODE SIMU...' % (self.name) )
            
            self.N1=True
            self.N2=False
            self.N3=False
            self.cmd=False
            state=0
            
            while True:
                self.valid=True

                if self.cmd==True:
                    state+=1
                    if state>30:
                        state=30
                else:
                    state-=1
                    if state<0:
                        state=0
                
                self.N1=True if state>8 else False
                self.N2=True if state>18 else False
                self.N3=True if state>25 else False
                self.RSSI=65530
                self.PWR=5

                time.sleep(1)


def resetkey(p,key,val,expire):
    p.set(key,val)
    p.expire(key,expire)

APP='wiio'

def set_app_var(p,vname,val,tmt):
    k='%s.%s' % (APP,vname)
    p.set(k,val)
    if tmt!=None:
        p.expire(k,tmt)

def app_mod_var(p,mname,vname,val,tmt):
    k='%s.%s.%s' % (APP,mname,vname)
    p.set(k,val)
    if tmt!=None:
        p.expire(k,tmt)

def get_app_var(p,vname):
    k='%s.%s' % (APP,vname)
    return p.get(k)

def update_wdg_var(p,vname,tmt):
    k='%s.%s' % (APP,vname)
    p.expire(k,tmt)

def update_mod_wdg_var(p,mname,vname,tmt):
    k='%s.%s.%s' % (APP,mname,vname)
    p.expire(k,tmt)


def app_get_mod_var(p,mname,vname):
    k='%s.%s.%s' % (APP,mname,vname)
    return p.get(k)

if __name__=='__main__':
    
    principal=WifiIoMgr('principal','192.168.3.210')
    paul=WifiIoMgr('paul','192.168.3.202')
    reduit=WifiIoMgr('reduit','192.168.3.203')
    
    modules=[principal,paul,reduit]

    r = redis.Redis(host='192.168.3.200', port=6379, db=0)

    ks=r.keys('wiio.*')
    p=r.pipeline()
    for k in ks:
        p.delete(k)
    p.execute()

    on=False    
    set_app_var(r,'alive',1,TMT)
    
    r.delete('wiio.modules')
    p=r.pipeline()
    for m in modules:
        r.sadd('%s.modules' % (APP),m.name)

        app_mod_var(p,m.name,'name',m.name,TMT)
        app_mod_var(p,m.name,'ip','',TMT)
        app_mod_var(p,m.name,'port','',TMT)
        app_mod_var(p,m.name,'cmd',0,TMT)
        app_mod_var(p,m.name,'n1',1,TMT)
        app_mod_var(p,m.name,'n2',1,TMT)
        app_mod_var(p,m.name,'n3',1,TMT)
        app_mod_var(p,m.name,'rssi',0,TMT)
        app_mod_var(p,m.name,'pwr',0,TMT)
        app_mod_var(p,m.name,'valid',0,TMT)

    p.execute()
    
    while True:            
        p = r.pipeline()
        for m in modules:
            app_mod_var(p,m.name,'name',m.name,TMT)
            app_mod_var(p,m.name,'ip',m.ip,TMT)
            app_mod_var(p,m.name,'port',m.port,TMT)
            
            app_mod_var(p,m.name,'n1',1 if m.N1==True else 0,TMT)
            app_mod_var(p,m.name,'n2',1 if m.N2==True else 0,TMT)
            app_mod_var(p,m.name,'n3',1 if m.N3==True else 0,TMT)
            
            app_mod_var(p,m.name,'rssi',m.RSSI,TMT)
            app_mod_var(p,m.name,'pwr',m.PWR,TMT)

            app_mod_var(p,m.name,'valid',1 if m.valid==True else 0,TMT)
        
        set_app_var(p,'alive',1,TMT)
        res=p.execute()        
            
        time.sleep(1)

        on=get_app_var(r,'on')            
        if on==b'1':
            if r.pttl('%s.on' % (APP))==-1:
                update_wdg_var(r,'on',TMT)
            on=True
        else:            
            on=False
            
        for m in modules:
            cmd=True if app_get_mod_var(r,m.name,'cmd')==b'1' else False
            if cmd==True and on==True:
                if r.pttl('%s.%s.cmd' % (APP,m.name))==-1:
                    update_mod_wdg_var(r,m.name,'cmd',TMT)                    
                m.cmd=True                
            else:
                m.cmd=False
           
