#!/usr/bin/python3
import paho.mqtt.client as mqtt
import redis
import time
import re
import json
import datetime

DEBUG_CYCLE=False
DEBUG_MQTT=True

class RdApp:
    TMT = 5
    
    def __init__(self,ip,port=6379):
        self.r = redis.Redis(host=ip, port=port, db=0, decode_responses=True)

    def kApp(self):
        return 'wiio'

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

    def get_mod_var_int(self,nmod,nvar):
        res=self.get_mod_var(nmod,nvar)
        if res==None or res.isnumeric()==False:
            return 0
        else:
            return int(res)

    def set_mod_var_bool(self,nmod,nvar,val,tmt=None):
        self.set_mod_var(nmod,nvar,1 if val==True else 0,tmt)

    def del_mod_var(self,nmod,nvar):
        k=self.kModVar(nmod,nvar)
        self.r.delete(k)
        

class RdWiioSrv(RdApp):
    def __init__(self,ip,port=6379):
        super(RdWiioSrv,self).__init__(ip,port)

        self.client = mqtt.Client()
        self.modules=set()
        
        ks=self.r.keys('%s.*' % (self.kApp()))
        p=self.r.pipeline()
        for k in ks:
            p.delete(k)
        p.execute()

        self.set_app_var('alive',1)
        #self.set_app_var('on',1,None)

        self.r.delete('%s.modules' % (self.kApp()))

    def on_connect(self,client, userdata, flags, rc):
        print("Connected with result code "+str(rc))

        client.subscribe("/wifiio/log/#")
        client.subscribe("/wifiio/data/#")

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
        
    def update_data(self,name,data):
        self.set_mod_var(name,'name',name,None)
        
        self.update_mod_var_bool(name,data,'cmd')
        self.update_mod_var_bool(name,data,'n1')
        self.update_mod_var_bool(name,data,'n2')
        self.update_mod_var_bool(name,data,'n3')

        self.update_mod_var_int(name,data,'rssi')
        self.update_mod_var_int(name,data,'pwr')
                            
        self.set_mod_var(name,'valid',1)

        now=datetime.datetime.now()
        self.set_mod_var(name,'date',now.isoformat(),None)
        
        
    pTopic=re.compile(r'/wifiio/data/([a-z]+)')
    def on_message(self,client,userdata,msg):
        if DEBUG_MQTT==True:
            print(msg.topic+" "+str(msg.payload))
        
        m=RdWiioSrv.pTopic.match(msg.topic)
        if m!=None:
            name=m.group(1)

            try:
                js=json.loads(msg.payload)
                self.update_data(name,js)
                print('-> Recv %s (%s)' % (name,js['cmd']))
                
                if name not in self.modules:
                    try:
                        self.r.sadd('%s.modules' % (self.kApp()),name)
                        self.modules.add(name)
                        print('Add %s' % name)
                    except Exception as ex:
                        print(ex)
            except:
                if DEBUG_MQTT==True:
                    print('Message incorrect')
                pass

            sleep=self.get_app_var_bool('sleep')
            if sleep==True and self.get_mod_var_bool(name,'sleep')==False:
                print('Envoi sleep a %s' % (name))
                self.client.publish("/wifiio/cmd/%s" % name,"sleep");
                self.set_mod_var_bool(name,'sleep',True,3000)

    def start(self):
        oldOn=False
        
        print('Start jarduino Wemos MQTT server...')
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.client.connect("192.168.3.200", 1883, 60)

        self.client.loop_start()

        print('Start program...');
        while True:
            time.sleep(1)

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

            slp=self.get_app_var_bool('sleep')
            
            for n in self.modules:
                if slp==True:
                    self.client.publish("/wifiio/cmd/%s" % n,"sleep");
                    continue
                
                v=self.get_mod_var(n,'to_cmd')
                if v!=None and v=='1':
                    self.client.publish("/wifiio/cmd/%s" % n,"on");
                    if DEBUG_CYCLE==True:
                        print('pub %s on' % n)
                else:
                    self.client.publish("/wifiio/cmd/%s" % n,"off");
                    if DEBUG_CYCLE==True:
                        print('pub %s off' % n)

                val=self.get_mod_var(n,'valid')
                if val==None:                    
                    print("Loose %s!" % n)
                    self.set_mod_var(n,'valid',0,None)

            if slp==True:
                self.set_app_var_bool('on',False,None)                
        

if __name__=='__main__':
    srv=RdWiioSrv('192.168.3.200')
    srv.start()





