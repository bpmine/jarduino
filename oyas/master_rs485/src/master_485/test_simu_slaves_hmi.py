import tkinter as tk
from tkinter import ttk
from threading import Thread
import threading
import serial.tools.list_ports
import time
import random
import serial
import re
import os

class Comm:
    def getPort(self):
        ports = list(serial.tools.list_ports.comports())
        if len(ports) == 1:
           return ports[0].device
        
        if os.path.exists('simu_slaves.txt'):
            with open('simu_slaves.txt', 'r') as file:
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
                    with open('simu_slaves.txt', 'w') as file:
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

        
    def calcCS(self,buff):
        cs=0
        for b in range(0,len(buff)):
            #print('%d' % ord(buff[b]))
            cs=cs+ord(buff[b])

        return cs % 256

    def decodeInfo(self,msg):
        pCmdResp=re.compile(r'\x01([0-9A-F]{2})([po])([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{4})([0-9A-F]{0,4})([0-9A-F]{2})\x02')
        print('Recv> %s' % msg)
        txt=str(msg,'ascii')
        m=pCmdResp.match(txt)
        if m!=None:
            size=int(m.group(1),16)
            typ=m.group(2)

            obj={
                'type':typ,
                'addr':m.group(3),
                'status':m.group(4),
                'tick':int(m.group(5),16),
                'temp':int(m.group(6),16),
                'hum':int(m.group(7),16),
                'volt':(float)(int(m.group(8),16)/10),
                'time':int(m.group(9),16),
                'errs':int(m.group(10),16)
                }

            if typ=='p':
                obj['flow']=int(m.group(11),16)
                cs=int(m.group(12),16)
            elif typ=='o':
                cs=int(m.group(12),16)
            else:
                print('ERROR: Incorrect type %s' % (typ))
                return None

            calc_cs=self.calcCS(txt[3:-3])

            if (calc_cs!=cs):
                print('ERREUR: CS Incorrect %02X recu au lieu de %02X attendu' % (calc_cs,cs))
                return None

            return obj

        return None

  
    def sendInfo(self,addr,inf):
        if addr==1:
            msg='\x011Dp%02X%02X%04X%02X%02X%02X%04X%04X%04X%02X\x02' % (
                addr,
                inf['status'],
                inf['tick'],
                inf['temp'],
                inf['hum'],
                inf['volt'],
                inf['time'],
                inf['errors'],
                inf['flow'],
                0
                )
        else:
            msg='\x0119o%02X%02X%04X%02X%02X%02X%04X%04X%02X\x02' % (
                addr,
                inf['status'],
                inf['tick'],
                inf['temp'],
                inf['hum'],
                inf['volt'],
                inf['time'],
                inf['errors'],
                0
                )

        calc_cs=self.calcCS(msg[3:-3])
        stcs='%02X' % (calc_cs)
        new_msg=msg[:-3]+stcs[0]+stcs[1]+msg[-1]
        msg=new_msg.encode('ascii')

        print('Send> %s' % msg)
        
        #res=decodeInfo(msg)
        #print(res)
        if inf['status']&0x02:
            print(f'{addr} ENABLED')

        if self.ser!=None:
            self.ser.write(msg)            
    
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

    def decode_cmds(self,msg):    
        pDecCmds=re.compile(r'\x0109m([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]{2})\x02')
        m=pDecCmds.match(msg)
        if m!=None:        
            cmds=int(m.group(1),16)
            addr=int(m.group(2),16)
            cs=int(m.group(3),16)
            calc_cs=self.calcCS(msg[3:-3])
            if calc_cs!=cs:
                print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
                return None

            return {
                'type':'cmds',
                'cmds':cmds,
                'addr':addr
                }
        else:
            return None


    def decode_raz(self,msg):
        pDecRaz=re.compile(r'\x0105([a-z])([0-9A-F]{2})([0-9A-F]{2})\x02')
        m=pDecRaz.match(msg)
        if m!=None:                
            tp=m.group(1)
            addr=int(m.group(2),16)
            cs=int(m.group(3),16)
            calc_cs=self.calcCS(msg[3:-3])
            if calc_cs!=cs:
                print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
                return None

            if tp=='t':
                typ='raz_time'
            elif tp=='z':
                typ='raz_errs'
            else:
                return None
                
            return {
                'type':typ,
                'addr':addr            
                }
        else:
            return None


    def decode_ping(self,msg):
        pDecRaz=re.compile(r'\x0107i([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})\x02')
        m=pDecRaz.match(msg)
        if m!=None:                
            addr=int(m.group(1),16)
            val=int(m.group(2),16)
            cs=int(m.group(3),16)
            calc_cs=self.calcCS(msg[3:-3])
            if calc_cs!=cs:
                print(f'ERROR CS {cs} recu et {cs_calc} calcule!')
                return None
                
            return {
                'type':'ping',
                'addr':addr,
                'val':val
                }
        else:
            return None

    
    def decode_frame(self,frame):    
        msg=str(frame,'ascii')
        print(msg)
        res=self.decode_cmds(msg)
        if res!=None:
            return res

        res=self.decode_raz(msg)
        if res!=None:
            return res

        res=self.decode_ping(msg)
        if res!=None:
            return res

        return None


    def treat_cmds(self,obj,datas):
        addr=obj['addr']
        cmds=obj['cmds']
        
        for at,inf in datas.items():
            msk=1<<at
            if (cmds&msk)==msk:
                inf['status']|=0x3
            else:
                inf['status']&=~0x3

            if at==addr:
                self.sendInfo(at,inf)
                break

        if addr==ord('S'):
            print('SYNC')
            return True
            
        return False

    def treat_raz(self,obj,datas):
        addr=obj['addr']
        msg=obj['type']
        
        if addr<len(infos):                        
            if msg=='raz_time':
                print(f'Raz TIME @%1X' % (addr))
                datas[addr]['time']=0
            elif msg=='raz_errs':
                print(f'Raz ERR @%1X' % (addr))
                datas[addr]['errors']=0
            else:
                print(f'Raz ?')


    def treat_ping(self,obj):
        print(obj)
        addr=obj['addr']
        val=obj['val']
        
        msg='\x0107y%02X%02X%02X\x02' % (
                addr,
                val,
                0
                )

        calc_cs=self.calcCS(msg[3:-3])
        stcs='%02X' % (calc_cs)
        new_msg=msg[:-3]+stcs[0]+stcs[1]+msg[-1]
        msg=new_msg.encode('ascii')

        if self.ser!=None:
            self.ser.write(msg)
            print('Send Pong: %s' % (msg))


class App:
    def __init__(self, root, comm):
        self.root = root
        self.root.title("Liste d'Éléments")
        self.comm=comm

        # Collection des adresses
        self.addresses = [1,2,3,4,5,6,7,8,9]

        self.datas={}
        for a in self.addresses:
            self.datas[a]={
                'addr':a,
                'status':0,
                'tick':0,
                'temp':22,
                'hum':85,
                'volt':102,
                'time':0,
                'errors':0,
                'flow':0,
                'enabled':True
                }

        self.create_widgets()
        self.update_values()

        self.thread = threading.Thread(target=self.background_task)
        self.thread.start()        

    def create_widgets(self):
        # Créer une barre d'en-tête
        header_frame = ttk.Frame(self.root, padding="10")
        header_frame.pack(fill=tk.X)

        headers = ["Adresse", "Low", "High", "ON", "Tick", "Volt", "Temp", "Hum", "Time", "ERRS"]
        widths = [15, 4, 5, 4, 10, 10, 10, 10, 10, 10]  # Largeurs pour chaque colonne
        for i, (header, width) in enumerate(zip(headers, widths)):
            label = ttk.Label(header_frame, text=header, width=width, anchor="center")
            label.grid(row=0, column=i, padx=5)

        self.frames = []
        for address in self.addresses:
            frame = ttk.Frame(self.root, padding="10")
            frame.pack(fill=tk.X, pady=5)

            # Adresse (lecture seule)
            addr_label = ttk.Label(frame, text='Esclave @%d' % address, width=15, anchor="w")
            addr_label.grid(row=0, column=0)

            # Checkbox "Low"
            low_var = tk.BooleanVar()
            low_check = ttk.Checkbutton(frame, text="Low", variable=low_var)
            low_check.grid(row=0, column=1)

            # Checkbox "High"
            high_var = tk.BooleanVar()
            high_check = ttk.Checkbutton(frame, text="High", variable=high_var)
            high_check.grid(row=0, column=2)

            # Indicateur "ON"
            on_indicator = tk.Canvas(frame, width=20, height=20, bg="grey")
            on_indicator.grid(row=0, column=3, padx=5)

            # Editbox "Tick" (lecture seule)
            tick_var = tk.StringVar()
            tick_entry = ttk.Entry(frame, textvariable=tick_var, state="readonly", width=10)
            tick_entry.grid(row=0, column=4, padx=5)

            # Editbox "Volt" (écriture possible)
            volt_var = tk.StringVar()
            volt_entry = ttk.Entry(frame, textvariable=volt_var, width=10)
            volt_entry.grid(row=0, column=5, padx=5)

            # Editbox "Temp" (écriture possible)
            temp_var = tk.StringVar()
            temp_entry = ttk.Entry(frame, textvariable=temp_var, width=10)
            temp_entry.grid(row=0, column=6, padx=5)

            # Editbox "Hum" (écriture possible)
            hum_var = tk.StringVar()
            hum_entry = ttk.Entry(frame, textvariable=hum_var, width=10)
            hum_entry.grid(row=0, column=7, padx=5)

            # Editbox "Time" (lecture seule)
            time_var = tk.StringVar()
            time_entry = ttk.Entry(frame, textvariable=time_var, state="readonly", width=10)
            time_entry.grid(row=0, column=8, padx=5)

            # Editbox "ERRS" (lecture seule)
            errs_var = tk.StringVar()
            errs_entry = ttk.Entry(frame, textvariable=errs_var, state="readonly", width=10)
            errs_entry.grid(row=0, column=9, padx=5)

            temp_var.set(25)
            hum_var.set(40)
            volt_var.set(125)
            
            self.frames.append({
                "low_var": low_var,
                "high_var": high_var,
                "on_indicator": on_indicator,
                "tick_var": tick_var,
                "volt_var": volt_var,
                "temp_var": temp_var,
                "hum_var": hum_var,
                "time_var": time_var,
                "errs_var": errs_var,
            })

    def update_values(self):

        self.addresses
        for i,frame in enumerate(self.frames):
            at=self.addresses[i]
            if at in self.datas:
                tck=self.datas[at]['tick']
                tck+=random.randint(500,1000)
                if tck>65535:
                    tck=0;
                frame["tick_var"].set(str(tck))
                
                if (self.datas[at]['status']&2) == 2:
                    self.datas[at]['time']+=1                    
                    frame["on_indicator"].config(bg="green")
                else:
                    frame["on_indicator"].config(bg="red")

                frame["time_var"].set(str(self.datas[at]['time']))
                frame["errs_var"].set(str(self.datas[at]['errors']))

                self.datas[at]['volt']=int(frame["volt_var"].get())
                self.datas[at]['temp']=int(frame["temp_var"].get())
                self.datas[at]['hum']=int(frame["hum_var"].get())

                if frame["low_var"].get()==True:
                    self.datas[at]['status']|=0x04
                else:
                    self.datas[at]['status']&=~0x04

                if frame["high_var"].get()==True:
                    self.datas[at]['status']|=0x08
                else:
                    self.datas[at]['status']&=~0x08
                            
            # Simuler des valeurs aléatoires pour les démonstrations
            #frame["tick_var"].set(f"{random.randint(0, 100)}")
            #frame["volt_var"].set(f"{random.uniform(0, 5):.2f}")
            #frame["temp_var"].set(f"{random.uniform(15, 30):.2f}")
            #frame["hum_var"].set(f"{random.uniform(30, 60):.2f}")
            #frame["time_var"].set(f"{random.uniform(0, 1000):.2f}")
            #frame["errs_var"].set(f"{random.randint(0, 10)}")
            

        # Appeler cette fonction à nouveau après 1 seconde
        self.root.after(1000, self.update_values)

    def background_task(self):
        print('Demarrage de la tache de fond..')
        while True:
            frame=self.comm.read_frame()
            if frame:
                obj=self.comm.decode_frame(frame)
                if obj==None:
                    continue

                if obj['type']=='cmds':
                    self.comm.treat_cmds(obj,self.datas)
                    continue
                elif obj['type'] in ['raz_time','raz_errs']:
                    self.comm.treat_raz(obj,self.datas)
                    continue
                elif obj['type']=='ping':
                    time.sleep(0.05);
                    self.comm.treat_ping(obj)
                    continue        

def main():
    comm=Comm()
    root = tk.Tk()
    app = App(root,comm)
    root.mainloop()

if __name__ == "__main__":
    main()
