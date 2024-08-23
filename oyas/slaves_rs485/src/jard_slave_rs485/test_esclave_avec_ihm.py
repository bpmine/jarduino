import tkinter as tk
from tkinter import ttk
import threading
import time
import serial
import re

PORT='COM14'
#PORT='COM21'

def calcCS(buff):
    cs=0
    for b in range(0,len(buff)):
        #print('%d' % ord(buff[b]))
        cs=cs+ord(buff[b])

    return cs % 256

def decodeInfo(msg):
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

        calc_cs=calcCS(txt[3:-3])

        if (calc_cs!=cs):
            print('ERREUR: CS Incorrect %02X recu au lieu de %02X attendu' % (calc_cs,cs))
            return None

        return obj

    return None


def sendCmd(ser,cmds,addr):
    cmd=''
    cmd+=chr(1)
    cmd+="09"
    cmd+='m'
    cmd+='%04X' % (cmds)
    cmd+='%02X' % (addr)
    dta=cmd[3:]
    cs=calcCS(dta)
    cmd+='%02X' % cs
    cmd+=chr(2)

    req=bytes(cmd,'ascii')
    print('Send %s' % (req))
    ser.write(req)
    msg=ser.read_until(b'\x02')
    if msg!=b'':
        obj=decodeInfo(msg)
        return obj

    return None

class MyApp:
    def __init__(self, root):
        self.root = root
        self.root.title("IHM Application")
        self.root.geometry("250x300")
        self.root.resizable(False, False)
        self.root.wm_attributes("-topmost", 1)  # Always on top

        # Address input
        self.address_label = tk.Label(root, text="Address:")
        self.address_label.pack(pady=5)
        self.address_entry = tk.Entry(root, width=50)
        self.address_entry.pack(pady=5)
        self.address_entry.insert(0, '6')

        # Checkbox for "cmds"
        self.cmds_var = tk.BooleanVar()
        self.cmds_check = tk.Checkbutton(root, text="Activation cmds", variable=self.cmds_var)
        self.cmds_check.pack(pady=5)
        self.cmds_var.set(False)

        # Checkbox for "sync"
        self.sync_var = tk.BooleanVar()
        self.sync_check = tk.Checkbutton(root, text="Activation sync", variable=self.sync_var)
        self.sync_check.pack(pady=5)
        self.sync_var.set(False)

        # Checkbox for "ON/OFF"
        self.on_var = tk.BooleanVar()
        self.on_check = tk.Checkbutton(root, text="ON/OFF", variable=self.on_var)
        self.on_check.pack(pady=5)
        self.on_var.set(False)

        # Boolean indicators (low and high)
        self.high_label = tk.Label(root, text="High:", bg="red", width=10)
        self.high_label.pack(pady=5)
        self.low_label = tk.Label(root, text="Low:", bg="red", width=10)
        self.low_label.pack(pady=5)

        # Flow
        self.flow = tk.Label(root, text="", fg="black")
        self.flow.pack(pady=5)        

        # Non-editable text label
        self.info_label = tk.Label(root, text="", fg="blue")
        self.info_label.pack(pady=5)        

        # Start background thread
        self.running = True
        self.thread = threading.Thread(target=self.background_task)
        self.thread.start()

        # Close button
        self.close_button = tk.Button(root, text="Close", command=self.close)
        self.close_button.pack(pady=5)

    def close(self):
        self.running = False
        self.thread.join()
        self.root.destroy()

    def background_task(self):
        ser= serial.Serial(port=r"\\.\%s" % PORT,stopbits = 1, bytesize = 8, parity='N',baudrate= 9600,timeout=0.5)

        time.sleep(2)
        res=ser.read(100)
        
        while self.running:
            addr = 0 if self.address_entry.get()=='' else int(self.address_entry.get())
            cmds_active = self.cmds_var.get()
            sync_active = self.sync_var.get()
            on=self.on_var.get()

            if addr>0 and addr<15:
                if on==True:
                    cmd=1<<addr
                else:
                    cmd=0
                    
                if cmds_active==True:
                    obj=sendCmd(ser,cmd,addr)
                    if obj!=None:
                        print(obj)
                        st=int(obj['status'],16)
                        if st&0x04:
                            low_state=True
                        else:
                            low_state=False
                            
                        if st&0x08:
                            high_state=True
                        else:
                            high_state=False

                        if 'flow' in obj:
                            self.flow.config(text=obj['flow'])
                        else:
                            self.flow.config(text='')
                        
                        self.update_bool_states(low_state, high_state)
                        self.info_label.config(text=f"{obj['temp']}°C {obj['hum']}% {obj['volt']}V {obj['time']}s {obj['errs']} errors {obj['tick']}ms")

                if sync_active==True:
                    sendCmd(ser,cmd,ord('S'))
                
            time.sleep(1)

        ser.close()
            

    def update_bool_states(self, low_state, high_state):
        self.low_label.config(bg="green" if low_state else "red")
        self.high_label.config(bg="green" if high_state else "red")

if __name__ == "__main__":
    root = tk.Tk()
    app = MyApp(root)
    root.mainloop()
