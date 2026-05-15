import time
import paho.mqtt.client as mqtt
from datetime import datetime
import json


client = mqtt.Client()
client.connect("192.168.3.200", 1883, 60)


modules_datas={
      'barbec':{
            'type':'data',
            'slaves':0x3F,
            'comms':0x3F,
            'cmds':0x00,
            'ons':0x00,
            'lows':0x1F,
            'highs':0x04,
            'date':'2026-05-15T18:00:00Z'
      }
      
}


client=mqtt.Client()
client.connect("192.168.3.200", 1883, 60)

for nme,dta in modules_datas.items():
    client.publish(f"/oyas/data/{nme}",json.dumps(dta).encode('ASCII'))

client.loop_start()



