#!/usr/bin/python3
import paho.mqtt.client as mqtt
import redis
import time
import re
import json
import datetime
import threading

DEBUG_CYCLE=False
DEBUG_MQTT=True

class RdApp:
    TMT = 5
    
    def __init__(self,ip,port=6379):
        self.r = redis.Redis(host=ip, port=port, db=0, decode_responses=True)

    def kApp(self):
        return 'oyas'

    def kMod(self,nmod):
        return '%s.%s' % (self.kApp(),nmod)

    def kModVar(self,nmod,nvar):
        return '%s.%s' % (self.kMod(nmod),nvar)

    def set_app_var(self,nvar,val,tmt=TMT):
        k='%s.%s' % (self.kApp(),nvar)
        self.r.set(k,val)
        if tmt!=None:
            self.r.expire(k,tmt)

    def get_app_var(self,nvar):
        k='%s.%s' % (self.kApp(),nvar)
        return self.r.get(k)

    def set_app_var_bool(self,nvar,val,tmt=TMT):
        self.set_app_var(nvar,1 if val==True else 0,tmt)

    def get_app_var_bool(self,nvar):
        res=self.get_app_var(nvar)
        if res==None:
            return False
        else:
            return True if res=='1' else False

    def set_mod_var(self,nmod,nvar,val,tmt=TMT):
        k=self.kModVar(nmod,nvar)
        self.r.set(k,val)
        if tmt!=None:
            self.r.expire(k,tmt)        

    def get_mod_var(self,nmod,nvar):
        k=self.kModVar(nmod,nvar)
        return self.r.get(k)

    def get_mod_var_bool(self,nmod,nvar):
        res=self.get_mod_var(nmod,nvar)
        if res==None:
            return False
        else:
            return True if res=='1' else False

    def get_mod_var_int(self,nmod,nvar,defaut=None):
        res=self.get_mod_var(nmod,nvar)
        if res==None: 
            return defaut
        else:
            try:
                return int(res)
            except:
                return 0

    def set_mod_var_bool(self,nmod,nvar,val,tmt=None):
        self.set_mod_var(nmod,nvar,1 if val==True else 0,tmt)

    def set_mod_var_int(self,nmod,nvar,val,tmt=None):
        self.set_mod_var(nmod,nvar,val,tmt)

    def del_mod_var(self,nmod,nvar):
        k=self.kModVar(nmod,nvar)
        self.r.delete(k)

    def kSlaveVar(self,nmod,addr,nvar):
        return '%s.%s.%s' % (self.kMod(nmod),addr,nvar)

    def set_slave_var(self,nmod,addr,nvar,value,tmt=TMT):
        k=self.kSlaveVar(nmod,addr,nvar)
        self.r.set(k,value)
        if tmt!=None:
            self.r.expire(k,tmt)

    def del_slave_var(self,nmod,addr,nvar):
        k=self.kSlaveVar(nmod,addr,nvar)
        self.r.delete(k)

    def get_slave_var(self,nmod,addr,nvar):
        k=self.kSlaveVar(nmod,addr,nvar)
        return self.r.get(k)       

    def get_slave_var_bool(self,nmod,addr,nvar):
        res=self.get_slave_var(nmod,addr,nvar)
        if res==None:
            return False
        else:
            return True if res=='1' else False

    def get_slave_var_int(self,nmod,addr,nvar):
        res=self.get_slave_var(nmod,addr,nvar)
        if res==None or res.isnumeric()==False:
            return 0
        else:
            return int(res)
                

class RdOyasSrv(RdApp):
    def __init__(self,ip,port=6379):
        super(RdOyasSrv,self).__init__(ip,port)

        self.client = mqtt.Client()
        self.modules=set()
        self.slaves={}
        
        ks=self.r.keys('%s.*' % (self.kApp()))
        p=self.r.pipeline()
        for k in ks:
            p.delete(k)
        p.execute()

        self.set_app_var_bool('alive',True,10)
        #self.set_app_var('on',1,None)

        self.r.delete('%s.modules' % (self.kApp()))

    def on_connect(self,client, userdata, flags, rc):
        print("Connected with result code "+str(rc))

        client.subscribe("/oyas/log/#")
        client.subscribe("/oyas/data/#")

    def update_mod_var_bool(self,name,data,key):
        if key in data:
            self.set_mod_var(name,key,1 if data[key]==True else 0,None)
        else:
            self.del_mod_var(name,key)

    def update_mod_var_int(self,name,data,key):
        if key in data:
            self.set_mod_var(name,key,data[key],None)
        else:
            self.del_mod_var(name,key)
            
    def update_slave_var_bool(self,name,addr,data,key):
        
        if key in data:
            self.set_slave_var(name,addr,key,1 if data[key]==True else 0,None)
        else:
            self.del_slave_var(name,addr,key)

    def update_slave_var_int(self,name,addr,data,key):
        if key in data:
            self.set_slave_var(name,addr,key,data[key],None)
        else:
            self.del_slave_var(name,addr,key)

    def send_mod_ack(self,name_module):
        js={'req':'ack'}
        self.client.publish("/oyas/cmd/%s" % name_module,json.dumps(js).encode('ASCII'))

    def send_mod_config(self,name_module,slaves,bigs):
        js={'req':'setcfg'}
        if slaves!=None:
            js['slaves']=slaves;
        if bigs!=None:
            js['slaves']=bigs;
        
        self.client.publish("/oyas/cmd/%s" % name_module,json.dumps(js).encode('ASCII'))

    def send_mod_cmds(self,name_module,cmds,ctrl,stop_highs=None):
        js={'req':'cmds','cmds':cmds,'ctrl':ctrl}
        if stop_highs!=None:
            js["highs"]=stop_highs
        self.client.publish("/oyas/cmd/%s" % name_module,json.dumps(js).encode('ASCII'))

    def update_data(self,name,data):
        self.set_mod_var(name,'name',name,None)

        if (data["type"]=='data'):
            self.update_mod_var_int(name,data,'slaves')
            self.update_mod_var_int(name,data,'comms')
            self.update_mod_var_int(name,data,'cmds')
            self.update_mod_var_int(name,data,'ons')
            self.update_mod_var_int(name,data,'lows')
            self.update_mod_var_int(name,data,'highs')
            self.update_mod_var_int(name,data,'bigs')
            self.set_mod_var(name,'date',data['date'],None)                    
                            
        self.set_mod_var(name,'valid',1)

        now=datetime.datetime.now()
        self.set_mod_var(name,'date_mqtt',now.isoformat(),None)
        
        
    pTopic=re.compile(r'/oyas/data/([a-z]+)')
    def on_message(self,client,userdata,msg):
        if DEBUG_MQTT==True:
            print(msg.topic+": "+str(msg.payload))
        
        m=RdOyasSrv.pTopic.match(msg.topic)
        if m!=None:
            name=m.group(1)

            try:
                js=json.loads(str(msg.payload,'ascii'))
                self.update_data(name,js)
                if DEBUG_MQTT==True:
                    print('-> Recv %s' % (name))
                
                #self.send_mod_ack(name)
                
                if name not in list(self.modules):
                    try:
                        print('ADD')
                        
                        self.r.sadd('%s.modules' % (self.kApp()),name)
                        self.modules.add(name)
                        print('Add %s' % name)
                    except Exception as ex:
                        print(ex)

                self.send_mod_config_ifneeded(name)

            except Exception as e:
                if DEBUG_MQTT==True:
                    print('Message incorrect: %s' % e)
                pass


    #def sync(self):
    #    print('Demarrage du Thread de synchro/requete...')
    #    while True:
    #        time.sleep(1)
    #        on=self.get_app_var_bool('on')
    #        if on==True:
    #            print('SYNC...')
    #            for n in self.modules:
    #                print(n)
    #                req={'req':'cmds','cmds':0,'ctrl':'0'}
    #                msg=json.dumps(req)
    #                self.client.publish(f'/oyas/cmd/{n}',msg)            

    def send_mod_config_ifneeded(self,name_module):
        on=self.get_app_var_bool('on')
        if on!=True:
            return;

        new_slaves=self.get_mod_var_int(name_module,'to_slaves')
        if new_slaves!=None:
            slaves=self.get_mod_var_int(name_module,'slaves')
            if slaves!=new_slaves:
                print(f'Envoi de la configuration des esclaves de {name_module}: {slaves:04X} -> {new_slaves:04X}')
                js={'req':'setcfg','slaves':new_slaves}
                self.client.publish("/oyas/cmd/%s" % name_module,json.dumps(js).encode('ASCII'))
                self.set_mod_var_int(name_module,'slaves',new_slaves)

        new_bigs=self.get_mod_var_int(name_module,'to_bigs')
        if new_bigs!=None:
            bigs=self.get_mod_var_int(name_module,'bigs')
            if bigs!=new_bigs:
                print(f'Envoi de la configuration des gros oyas à {name_module}: {bigs:04X} -> {new_bigs:04X}')
                js={'req':'setcfg','bigs':new_bigs}
                self.client.publish("/oyas/cmd/%s" % name_module,json.dumps(js).encode('ASCII'))
                self.set_mod_var_int(name_module,'bigs',new_bigs)


    def start(self):
        oldOn=False
        
        print('Start jarduino Oyas masters MQTT server...')
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.client.connect("192.168.3.200", 1883, 60)

        self.client.loop_start()

        #th = threading.Thread(target=self.sync)
        #th.start()

        #self.modules.add('barbec')
        self.set_app_var('on',0,None)
        print('Start program...');
        while True:
            time.sleep(1)
            self.set_app_var_bool('alive',True,10)

            on=self.get_app_var_bool('on')
            if on==None:
                on=False
                self.set_app_var('on',0,None)
                
            if oldOn!=on:                
                print("Mise en marche" if on==True else "Arrêt!")
                oldOn=on
            
            if on!=True:
                continue            

            if DEBUG_CYCLE==True:
                print('_'*40)
                print('Cycle:')
                for n in self.modules:
                    print(f'  - {n}')
            
            for n in self.modules:
                to_cmds=self.get_mod_var_int(n,'to_cmds')
                if to_cmds==None:
                    to_cmds=0

                stop_highs=None
                to_filling=self.get_mod_var_int(n,'to_filling')
                if to_filling!=None:
                    stop_highs=to_filling&0xFFFE;
                    to_cmds=to_filling;

                self.send_mod_cmds(n,to_cmds,True,stop_highs)
                
                #js={'req':'cmds','cmds':to_cmds,'ctrl':True}
                #if stop_highs!=None:
                #    js["highs"]=stop_highs                    
                #self.client.publish("/oyas/cmd/%s" % n,json.dumps(js).encode('ASCII'))

##                
##                v=self.get_mod_var(n,'to_cmd')
##                if v!=None and v=='1':
##                    self.client.publish("/oyas/cmd/%s" % n,"on");
##                    if DEBUG_CYCLE==True:
##                        print('pub %s on' % n)
##                else:
##                    self.client.publish("/oyas/cmd/%s" % n,"off");
##                    if DEBUG_CYCLE==True:
##                        print('pub %s off' % n)
##
##                val=self.get_mod_var(n,'valid')
##                if val==None:                    
##                    print("Loose %s!" % n)
##                    self.set_mod_var(n,'valid',0,None)
##
##            if slp==True:
##                self.set_app_var_bool('on',False,None)                
        

if __name__=='__main__':
    srv=RdOyasSrv('192.168.3.200')
    srv.start()





