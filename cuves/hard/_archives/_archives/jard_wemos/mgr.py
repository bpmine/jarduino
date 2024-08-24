from flask import Flask,request
import json
import pymodbus

try:
    from pymodbus.client import ModbusTcpClient as ModbusClient
except:
    try:
        from pymodbus.client.sync import ModbusTcpClient as ModbusClient
    except:
        pass
    
import time
from threading import Thread

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


class Rempli:
    def __init__(self,name,pmp,lvl_tgt,lvl_src):
        self.name=name;
        self.pmp=pmp
        self.lvl_tgt=lvl_tgt
        self.lvl_src=lvl_src
        self.on=False
        self.cons_tgt=0
        self.limit_src=0

    def start(self,cons_tgt,limit_src=1):
        self.cons_tgt=cons_tgt
        self.limit_src=limit_src
        self.on=True

    def stop(self):
        self.on=False
        self.cons_tgt=0
        self.limit_src=0
        self.pmp.cmd=False

    def run(self):
        if self.on==True:

            # Si le niveau de la source est donne, on arrete qd trop bas !
            src_too_low=False
            if self.lvl_src!=None:
                if self.lvl_src.valid==False:
                    src_too_low=True                    
                elif self.limit_src==1:
                    src_too_low=not self.lvl_src.N1
                elif self.limit_src==2:
                    src_too_low=not self.lvl_src.N2
                elif self.limit_src==3:
                    src_too_low=not self.lvl_src.N3                    

            # Sinon, on cherche a atteindre la consigne
            if src_too_low==False:
                if self.lvl_tgt!=None:
                    if self.lvl_tgt.valid==False:
                        self.pmp.cmd=False
                        
                    if self.cons_tgt==1:
                        self.pmp.cmd=not self.lvl_tgt.N1
                    elif self.cons_tgt==2:
                        self.pmp.cmd=not self.lvl_tgt.N2
                    elif self.cons_tgt==3:
                        self.pmp.cmd=not self.lvl_tgt.N3
                    else:
                        self.pmp.cmd=False
                else:
                    self.pmp.cmd=True
                    
            else:
                self.pmp.cmd=False

    def __str__(self):
        return ""


principal=WifiIoMgr('Principal','192.168.3.210')
paul=WifiIoMgr('Paul','192.168.3.202')
reduit=WifiIoMgr('Reduit','192.168.3.203')
barbec=WifiIoMgr('Barbec','192.168.3.204')
mgrs=[principal,paul,reduit,barbec]


rempli_reduit=Rempli('main_reduit',principal,reduit,principal)
rempli_paul=Rempli('main_paul',paul,paul,principal)
rempli_barbec=Rempli('reduit_barbec',reduit,None,reduit)
remplissages=[rempli_reduit,rempli_paul,rempli_barbec]

        
def process_remplissage():
    print('Start tache de fond des remplissages');
    
    #rempli_reduit.start(2,3)
    #rempli_barbec.start(2,3)

    while True:
        time.sleep(2)
        for r in remplissages:
            r.run()
        
t2 = Thread(target=process_remplissage, args=[])
t2.start()

app = Flask(__name__)

@app.route('/test', methods=['GET'])
def test():
    return 'ok'

@app.route('/rempli/<name>', methods=['GET'])
def rempli_set_on(name):
    if name==None:
        return "no name"

    rr=None
    for r in remplissages:
        if r.name==name:
            rr=r
            break

    if rr==None:
        return "Pas de remplissage trouve"
    
    on=request.args.get('on',default=None)    
    if on!=None:        
        if on=='1':
            rr.on=True
        else: 
            rr.stop()

        print('Change le ON de %s en %s' % (name,rr.on) )
        
    limit_src=request.args.get('limit_src',default=None)    
    if limit_src!=None:        
        rr.limit_src=int(limit_src)
        print('Change la limite de la source %s en %s' % (name,rr.limit_src) )

    cons_tgt=request.args.get('cons_tgt',default=None)    
    if cons_tgt!=None:        
        rr.cons_tgt=int(cons_tgt)
        print('Change la consigne de la destination %s en %s' % (name,rr.cons_tgt) )
        

    return "OK"

@app.route('/states', methods=['GET'])
def states():
    ret={
        'name':'jarduino'
        }

    ret['modules']=[]
    for m in mgrs:
        ret['modules'].append({
                'name':m.name,
                'cmd':m.cmd,
                'N1':m.N1,
                'N2':m.N2,
                'N3':m.N3,
                'RSSI':m.RSSI,
                'PWR':m.PWR,
                'ip':m.ip,
                'valid':m.valid
            })            

    ret['remplissages']=[]
    for m in remplissages:
        ret['remplissages'].append({
                'name':m.name,
                'cons_tgt':m.cons_tgt,
                'limit_src':m.limit_src,
                'on':m.on
            })

    return json.dumps(ret);


if __name__=='__main__':
    #app.debug = True
    app.run(host='0.0.0.0',port=5000,debug=False)
