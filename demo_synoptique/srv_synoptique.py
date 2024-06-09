#!/usr/bin/python3
import time
from flask import Flask, send_from_directory
from flask_socketio import SocketIO
import threading
import time
import requests
import datetime

##                'main': {
##                    'level': 2,
##                    'pompe': true,
##                    'state': 'on',
##                    'volt': 123,
##                    'db': 65521,
##                    'time': '2024-05-31T14:00:00Z'
##                },

#{'name': 'main', 'cmd': False, 'n1': True, 'n2': True, 'n3': True, 'rssi': 0, 'pwr': 966, 'valid': False, 'sleep': True, 'date': '2024-06-09T09:24:57.764627'}




class FetchThread(threading.Thread):
    def __init__(self,socketio):
        super().__init__()
        self.daemon = True
        self.datas=None
        self.socketio=socketio

    def level(self,n1,n2,n3):
        if n1==False and n2==False and n3==False:
            return 0
        elif n1==True and n2==False and n3==False:
            return 1
        elif n1==True and n2==True and n3==False:
            return 2
        elif n1==True and n2==True and n3==True:
            return 3
        else:
            return 0

    def run(self):
        while True:
            try:
                response = requests.get("http://192.168.3.200:5000/states")
                if response.status_code == 200:
                    self.datas= response.json()
                    if 'modules' in self.datas:
                        ret={}
                        mods=self.datas["modules"]
                        todo=['main','paul','reduit','barbec']
                        for t in todo:
                            
                            if t not in mods:
                                obj={
                                    'level':0,
                                    'pompe':False,
                                    'db':0,
                                    'volt':0,
                                    'time':datetime.datetime.now().strftime('%Y-%m-%dT%H:%M:%SZ'),
                                    'state':'sleep'
                                    }
                            else:
                                obj={
                                    'level':self.level(mods['n1'],mods['n2'],mods['n3']),
                                    'pompe':mods['cmd'],
                                    'db':mods['rssi'],
                                    'volt':mods['pwr'],
                                    'time':mods['date'].strftime('%Y-%m-%dT%H:%M:%SZ'),
                                    'state':'sleep' if mods['sleep']==True else 'on' if mods['valid']==True else 'off'
                                    }
                                
                            ret[t]=obj
                        
                        print(ret)
                        self.socketio.emit('data_update', ret)
                else:
                    print(f"Failed to retrieve data: {response.status_code}")
            except Exception as e:
                print(f"Error fetching data: {e}")
                
            time.sleep(60)  # Attendre 60 secondes avant de récupérer à nouveau


app = Flask(__name__)
socketio = SocketIO(app)
fetch_thread = FetchThread(socketio)

@app.route('/test', methods=['GET'])
def test():
    return 'ok'

@app.route('/')
def index():
    chemin_repertoire = r'C:\DEV\GITHUB\jarduino\demo_synoptique\www'
    return send_from_directory(chemin_repertoire, 'index.html')

@app.route('/<path:chemin_fichier>')
def serve_static(chemin_fichier):
    chemin_repertoire = r'C:\DEV\GITHUB\jarduino\demo_synoptique\www'
    return send_from_directory(chemin_repertoire, chemin_fichier)

if __name__ == '__main__':    
    fetch_thread.start()
    
    app.run(debug=True, host='0.0.0.0',port=5001,use_reloader=False)
