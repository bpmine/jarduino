import serial
import time
import re
import threading
import json
import serial.tools.list_ports
import os

class Comm:
    def getPort(self):
        ports = list(serial.tools.list_ports.comports())
        if len(ports) == 1:
           return ports[0].device
        
        if os.path.exists('simu_esp01.txt'):
            with open('simu_esp01.txt', 'r') as file:
                saved_port = file.read().strip()
                for port in ports:
                    if port.device == saved_port:
                        return saved_port
                    
            print(f"Le port enregistré '{saved_port}' n'est pas disponible.")
            
        print("Ports série disponibles:")
        for i, port in enumerate(ports):
            print(f"{i + 1}: {port.device} - {port.description}")

        while True:
            try:
                choice = int(input(f"Choisissez un port (1-{len(ports)}): "))
                if 1 <= choice <= len(ports):
                    chosen_port = ports[choice - 1].device
                    with open('simu_esp01.txt', 'w') as file:
                        file.write(chosen_port)
                    return chosen_port
                else:
                    print(f"Choix invalide. Veuillez entrer un nombre entre 1 et {len(ports)}.")
            except ValueError:
                print("Entrée invalide. Veuillez entrer un nombre.")

        # Aucun port disponible
        print("Aucun port série disponible.")
        return None

    def __init__(self):
        prt=self.getPort()
        if prt!=None:
            print('Ouverture du port %s' % (prt))
            self.ser= serial.Serial(port=r"\\.\%s" % prt,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)
        else:
            self.ser=None
    
    def read_frame(self):
        if self.ser==None:
            return None
        
        frame = bytearray()
        while True:
            byte = self.ser.read(1)
            if not byte:
                break
            if byte == b'\x01':
                frame = bytearray(b'\x01')
            elif frame:
                frame.append(byte[0])
                if byte == b'\x02':
                    return frame

    def send_ack(self):
        s='\x01'
        s+=json.dumps({"req":"ack"}) 
        s+='\x02'
        s=s.encode('ASCII')
        print('Send> ',s)
        self.ser.write(s)        

            
def sendCmd(ser,jsTxt):
    msg=''
    msg+=chr(1)
    msg+=jsTxt
    msg+=chr(2)

    req=bytes(msg,'ascii')
    print('Send %s' % (req))
    ser.write(req)
    msg=ser.read_until(b'\x02')
    if msg!=b'' and len(msg)>2:
        jsTxt=msg[1:-1]
        print(jsTxt)
        return json.loads(jsTxt)
    else:
        return None

def test():
    comm=Comm();

    while True:
        frame=comm.read_frame()
        if frame!=None:
            try:
                jsTxt=str(frame[1:-1],'ASCII')
                js=json.loads(jsTxt)
                print(js)
                if js['type'] in ['master','pump','oya','data']:
                    comm.send_ack()
                    
            except Exception as ex:
                print(ex)
            
            
        
    #sendCmd(ser,'{"req":"test"}')

    #sendCmd(ser,'{"req":"master"}')

    #sendCmd(ser,'{"req":"pump"}')
    
    #sendCmd(ser,'{"req":"oya","addr":"2"}')

    #res=sendCmd(ser,'{"req":"cmds","cmds":255,"ctrl":true}')
    #print(res)
    #time.sleep(4)
    #res=sendCmd(ser,'{"req":"cmds","cmds":0,"ctrl":false}')
    #print(res)


test()

