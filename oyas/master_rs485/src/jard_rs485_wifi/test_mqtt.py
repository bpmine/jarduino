from paho.mqtt import client as mqtt_client
import time
import json
import logging
import re

broker = '192.168.3.200'
port = 1883
topic = "/oyas/#"
client_id = f'test_oyas2'
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

pTopic=re.compile(r'^/oyas/data/(.+)$')
def on_message(client, userdata, msg):
    m=pTopic.match(msg.topic)
    if m!=None:
        node=m.group(1)
        #print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        try:
            js=json.loads(msg.payload.decode())
            print(js)
            if 'type' in js and js['type']=='data':
                ans=json.dumps({"req":"ack"})
                print('->Ack')
                client.publish(f'/oyas/cmd/{node}',ans)
        except Exception as ex:
            print(ex)

client = mqtt_client.Client(client_id)

client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.connect(broker, port)

client.subscribe(f"/oyas/data/{node}")
client.on_message = on_message

#req={'req':'cmds','cmds':0,'ctrl':False}
#msg=json.dumps(req)
#client.publish('/oyas/cmd/barbec',msg)

#req={'req':'master'}
#msg=json.dumps(req)
#client.publish(f'/oyas/cmd/{node}',msg)

#req={'req':'oya','addr':2}
#msg=json.dumps(req)
#client.publish(f'/oyas/cmd/{node}',msg)

client.loop_forever()
