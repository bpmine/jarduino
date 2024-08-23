from paho.mqtt import client as mqtt_client
import time
import json
import logging

broker = '192.168.3.200'
port = 1883
topic = "/oyas/#"
client_id = f'test_oyas'
node='reduit'

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)


FIRST_RECONNECT_DELAY = 1
RECONNECT_RATE = 2
MAX_RECONNECT_COUNT = 12
MAX_RECONNECT_DELAY = 60
def on_disconnect(client, userdata, rc):
    logging.info("Disconnected with result code: %s", rc)
    reconnect_count, reconnect_delay = 0, FIRST_RECONNECT_DELAY
    while reconnect_count < MAX_RECONNECT_COUNT:
        logging.info("Reconnecting in %d seconds...", reconnect_delay)
        time.sleep(reconnect_delay)

        try:
            client.reconnect()
            logging.info("Reconnected successfully!")
            return
        except Exception as err:
            logging.error("%s. Reconnect failed. Retrying...", err)

        reconnect_delay *= RECONNECT_RATE
        reconnect_delay = min(reconnect_delay, MAX_RECONNECT_DELAY)
        reconnect_count += 1
    logging.info("Reconnect failed after %s attempts. Exiting...", reconnect_count)

def on_message(client, userdata, msg):
    print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

client = mqtt_client.Client(client_id)

client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.connect(broker, port)

def animation():

    #req={'req':'cmds','cmds':0,'ctrl':False}
    #msg=json.dumps(req)
    #client.publish('/oyas/cmd/barbec',msg)

    while True:
    #for i in range(1,15):
        for cmd in [0x02,0x04,0x08,0x10,0x20,0x40,0x80]:
            print('%02x' % cmd)

            for t in range(1,20):
                print('sync')
                
                req={'req':'cmds','cmds':cmd,'ctrl':True}
                msg=json.dumps(req)
                client.publish(f'/oyas/cmd/{node}',msg)
                time.sleep(0.2)


            time.sleep(1)
        #input('>')

    #client.loop_forever()

def monitors():

    while True:
        #print('sync')
                    
        #req={'req':'cmds','cmds':0,'ctrl':True}
        #msg=json.dumps(req)
        #client.publish(f'/oyas/cmd/{node}',msg)
        time.sleep(1)

monitors()

    
