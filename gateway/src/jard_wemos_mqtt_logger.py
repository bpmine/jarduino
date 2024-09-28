#!/usr/bin/python3
import paho.mqtt.client as mqtt
import redis
import time
import re
import json
import datetime

fp=open('log.txt','a')

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("/wifiio/log/#")
    client.subscribe("/wifiio/data/#")
    client.subscribe("/wifiio/cmd/#")
        
    client.subscribe("/oyas/log/#")
    client.subscribe("/oyas/data/#")
    client.subscribe("/oyas/cmd/#")

def on_message(client,userdata,msg):
    now=datetime.datetime.now()
    sDte=now.strftime("%d/%m/%Y %H:%M:%S")

    lg='%s> %-20s: %s\n' % (sDte,msg.topic,msg.payload)
    fp.write(lg)
    print(lg,end='')
    
    
print('Start jarduino logger...')
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.3.200", 1883, 60)

client.loop_forever()





