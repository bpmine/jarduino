import paho.mqtt.client as mqtt
import time

# Configuration des brokers A et B
broker_a = "192.168.3.200"
port_a = 1883
topic_a_subscribe = ["/oyas/#", "/wifiio/#", "/test/#"]

broker_b = "35.181.147.175"
port_b = 1883
username='domotique'
password='domo38'

DEBUG=False

# Configuration des topics de publication sur le broker B
topic_b_oyas_cmd = "/send/oyas/cmd"
topic_b_wifio_cmd = "/send/wifiio/cmd"

# Variables pour maintenir les clients
client_a = None
client_b = None

def on_connect_a(client, userdata, flags, rc):
    print(f"Connecté au broker A avec le code {rc}")
    for topic in topic_a_subscribe:
        client.subscribe(topic)
        print(f"Abonnement au topic {topic} sur le broker A")

def on_message_a(client, userdata, msg):
    if DEBUG==True:
        print(f"Message reçu sur le broker A topic {msg.topic}: {msg.payload.decode()}")
        
    # Publier sur le broker B
    client_b.publish(msg.topic, msg.payload)
    if DEBUG==True:
        print(f"Message publié sur le broker B topic {msg.topic}: {msg.payload}")

def on_connect_b(client, userdata, flags, rc):
    print(f"Connecté au broker B avec le code {rc}")
    client.subscribe(topic_b_oyas_cmd)
    client.subscribe(topic_b_wifio_cmd)

def on_message_b(client, userdata, msg):
    if DEBUG==True:
        print(f"Message reçu sur le broker B topic {msg.topic}: {msg.payload.decode()}")
        
    if msg.topic.startsWith(topic_b_oyas_cmd):
        client_a.publish(msg.topic.replace(topic_b_oyas_cmd,'/oyas/cmd'), msg.payload)
        if DEBUG==True:
            print(f"Message publié sur le broker A topic {msg.topic}: {msg.payload}")
            
    elif msg.topic.startsWith(topic_b_wifio_cmd):
        client_a.publish(msg.topic.replace(topic_b_wifio_cmd,'/wifio/cmd'), msg.payload)
        if DEBUG==True:
            print(f"Message publié sur le broker A topic {msg.topic}: {msg.payload}")


def on_disconnect(client, userdata, rc):
    print(f"Déconnecté du broker B avec le code {rc}")

def setup_client(client, broker, port, on_connect, on_message=None):
    client.on_connect = on_connect
    if on_message:
        client.on_message = on_message
    client.connect(broker, port)
    client.loop_start()

def main():
    global client_a, client_b

    # Création des clients MQTT
    client_a = mqtt.Client()
    client_b = mqtt.Client()
    client_b.username_pw_set(username, password)

    # Configurer les callbacks pour le broker A
    client_a.on_connect = on_connect_a
    client_a.on_message = on_message_a
    setup_client(client_a, broker_a, port_a, on_connect_a, on_message_a)

    # Configurer les callbacks pour le broker B
    client_b.on_connect = on_connect_b
    client_b.on_disconnect = on_disconnect
    client_b.on_message = on_message_b
    setup_client(client_b, broker_b, port_b, on_connect_b)

    try:
        while True:
            time.sleep(1)  # Boucle principale pour maintenir les connexions
    except KeyboardInterrupt:
        print("Interruption par l'utilisateur. Déconnexion...")
    finally:
        client_a.loop_stop()
        client_b.loop_stop()
        client_a.disconnect()
        client_b.disconnect()

if __name__ == "__main__":
    main()
