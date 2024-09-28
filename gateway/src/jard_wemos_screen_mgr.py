#!/usr/bin/python3
import paho.mqtt.client as mqtt
import redis
import time
import re
import json
import datetime

DEBUG_MQTT=False

REDIS_IP='192.168.3.200'
MQTT_IP=REDIS_IP
MQTT_TOPIC_DATAS='/wifiio/data/#'
MQTT_TOPIC_BOOT='/wifiio/screen/boot'

class RdWiioScreen():
    def __init__(self,ip=REDIS_IP,port=6379):
        self.r = redis.Redis(host=ip, port=port, db=0, decode_responses=True)
        self.ip=ip
        self.client = mqtt.Client()
        self.mods={}
        
    def on_connect(self,client, userdata, flags, rc):
        print("Connected with result code "+str(rc))
        client.subscribe(MQTT_TOPIC_DATAS)
        client.subscribe(MQTT_TOPIC_BOOT)

    def read_mod_bool(self,mname,vname):
        k='wiio.%s.%s' % (mname,vname)
        return True if self.r.get(k)=='1' else False
        
        
    pTopic=re.compile(r'/wifiio/data/([a-z]+)')
    def on_message(self,client,userdata,msg):
        if DEBUG_MQTT==True:
            print(msg.topic+" "+str(msg.payload))

        # Sur reception d'un boot, on envoi tous les messages d'event des cuves
        if msg.topic==MQTT_TOPIC_BOOT:
            mqtt_nme=msg.payload
            if DEBUG_MQTT==True:
                print('Reception boot de %s' % (mqtt_nme))
                
            for nme in self.mods.keys():
                event={
                    'name':nme,
                    'n1':self.mods[nme]['n1'],
                    'n2':self.mods[nme]['n2'],
                    'n3':self.mods[nme]['n3'],
                    'cmd':self.mods[nme]['cmd']
                    }

                self.client.publish("/wifiio/screen/data",json.dumps(event))
                
            return

        # Sur reception d'une data, on propage l'event aux ecrans
        m=RdWiioScreen.pTopic.match(msg.topic)
        if m!=None:
            name=m.group(1)

            try:
                js=json.loads(msg.payload)

                force=False
                if name not in self.mods:
                    force=True
                    self.mods[name]={
                        'n1':False,
                        'n2':False,
                        'n3':False,
                        'cmd':False
                        }

                pn1=self.mods[name]['n1']
                pn2=self.mods[name]['n2']
                pn3=self.mods[name]['n3']
                pcmd=self.mods[name]['cmd']
                
                n1=js['n1']
                n2=js['n2']
                n3=js['n3']
                cmd=js['cmd']

                self.mods[name]['n1']=n1
                self.mods[name]['n2']=n2
                self.mods[name]['n3']=n3
                self.mods[name]['cmd']=cmd                

                event={
                    'name':name,
                    'n1':n1,
                    'n2':n2,
                    'n3':n3,
                    'cmd':cmd
                    }

                if n1!=pn1 or n2!=pn2 or n3!=pn3 or cmd!=pcmd or force==True:                
                    self.client.publish("/wifiio/screen/data",json.dumps(event))
                                
            except Exception as e:
                if DEBUG_MQTT==True:
                    print('Message incorrect %s' % (e))
                pass
               

    def start(self):
        oldOn=False
        
        print('Start jarduino Wemos MQTT server...')
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

        self.client.connect(MQTT_IP, 1883, 60)       

        self.client.loop_forever()

        print('Start program...');
        while True:
            time.sleep(1)

if __name__=='__main__':
    srv=RdWiioScreen(REDIS_IP)
    srv.start()





