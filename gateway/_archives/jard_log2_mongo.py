#!/usr/bin/python3
import paho.mqtt.client as mqtt
import redis
import time
import re
import json
import datetime
from pymongo import MongoClient
import json

cln=MongoClient()
db=cln.jarduino
col=db.oyas

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    #client.subscribe("/wifiio/log/#")
    #client.subscribe("/wifiio/data/#")
    #client.subscribe("/wifiio/cmd/#")
        
    client.subscribe("/oyas/log/#")
    client.subscribe("/oyas/data/#")
    client.subscribe("/oyas/cmd/#")

def on_message(client,userdata,msg):
    now=datetime.datetime.now()

    try:        
        jsTxt=str(msg.payload,'ascii')
        js=json.loads(jsTxt)
        js['date_mqtt']=now
        js['topic']=msg.topic

        col.insert_one(js)
    except Exception as ex:
        print(msg.payload)
        print(ex)
            
    
print('Start MONGO logger...')

client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.3.200", 1883, 60)

client.loop_forever()





